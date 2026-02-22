#include <cstring>
#include <chrono>
#include <memory>
#include "protocol/haier_protocol.h"

namespace haier_protocol
{

constexpr uint8_t MAX_PACKET_RETRIES = 9;
constexpr std::chrono::milliseconds DEFAULT_ANSWER_TIMEOUT = std::chrono::milliseconds(200);
constexpr std::chrono::milliseconds DEFAULT_COOLDOWN_INTERVAL = std::chrono::milliseconds(400);

#if HAIER_LOG_LEVEL > 3
  static std::chrono::steady_clock::time_point last_message_sent_;
#endif

ProtocolHandler::ProtocolHandler(ProtocolStream &stream) noexcept : ProtocolHandler(stream, MAX_FRAME_SIZE + 10)
{
}

ProtocolHandler::ProtocolHandler(ProtocolStream& stream, size_t buffer_size) noexcept : transport_(stream, buffer_size),
  message_handlers_map_(),
  answer_handlers_map_(),
  timeout_handlers_map_(),
  outgoing_messages_(),
  default_message_handler_(default_message_handler),
  default_answer_handler_(default_answer_handler),
  default_timeout_handler_(default_timeout_handler),
  state_(ProtocolState::IDLE),
  processing_message_(false),
  incoming_message_crc_status_(false),
  answer_sent_(false),
  last_message_type_(FrameType::UNKNOWN_FRAME_TYPE),
  answer_timeout_interval_(DEFAULT_ANSWER_TIMEOUT),
  cooldown_interval_(DEFAULT_COOLDOWN_INTERVAL)
{
  this->cooldown_time_point_ = std::chrono::steady_clock::time_point();
}

void ProtocolHandler::loop()
{
  this->transport_.read_data();
  this->transport_.process_data();
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  switch (this->state_)
  {
  case ProtocolState::IDLE:
    // Check incoming messages
    {
      size_t messagesCount = this->transport_.available();
      if (messagesCount > 1)
      {
        // Shouldn't get more than 1 message, drop all except last
        HAIER_LOGW("Incoming queue size %d (should be not more than 1). Dropping extra messages", messagesCount);
        this->transport_.drop(messagesCount - 1);
        messagesCount = 1;
      }
      if (messagesCount > 0)
      {
        TimestampedFrame frame;
        this->transport_.pop(frame);
        FrameType msg_type = (FrameType) frame.frame.get_frame_type();
        this->incoming_message_crc_status_ = frame.frame.get_use_crc();
        std::map<FrameType, MessageHandler>::const_iterator handler = this->message_handlers_map_.find(msg_type);
        this->processing_message_ = true;
        this->answer_sent_ = false;
        HandlerError hres;
        if (handler != this->message_handlers_map_.end())
          hres = handler->second(msg_type, frame.frame.get_data(), frame.frame.get_data_size());
        else
          hres = default_message_handler_(msg_type, frame.frame.get_data(), frame.frame.get_data_size());
        this->processing_message_ = false;
        if (hres != HandlerError::HANDLER_OK)
        {
          HAIER_LOGW("Message handler error, msg=%02X, err=%d", msg_type, hres);
        }
        else if (!this->answer_sent_)
        {
          HAIER_LOGW("No answer sent in incoming messages handler, message type %02X", msg_type);
        }
      }
      {
        if ((!this->outgoing_messages_.empty()) && (now >= this->cooldown_time_point_) && (now >= this->retry_time_point_))
        {
          // Ready to send next message
          OutgoingQueueItem &msg = this->outgoing_messages_.front();
          if (msg.number_of_retries > 0) {
            if (this->write_message_(msg.message, msg.use_crc))
            {
              this->last_message_type_ = msg.message.get_frame_type();
              if (msg.no_answer)
              {
                this->outgoing_messages_.pop();
                this->retry_time_point_ = now;
              }
              else
              {
                this->state_ = ProtocolState::WAITING_FOR_ANSWER;
                this->answer_time_point_ = now + this->answer_timeout_interval_;
                this->retry_time_point_ = now + msg.retry_interval;
              }
            }
            msg.number_of_retries--;
          } else {
            this->outgoing_messages_.pop();
            this->retry_time_point_ = now;
          }
        }
      }
    }
    break;
  case ProtocolState::WAITING_FOR_ANSWER:
    // Check for timeout, move to idle after timeout
    if (now > this->answer_time_point_)
    {
      // Answer timeout
      OutgoingQueueItem& msg = this->outgoing_messages_.front();
      if (msg.number_of_retries == 0) {
        // No more retries, remove message
        this->outgoing_messages_.pop();
        this->retry_time_point_ = now;
        HandlerError hres;
        std::map<FrameType, TimeoutHandler>::const_iterator handler = this->timeout_handlers_map_.find(this->last_message_type_);
        if (handler != this->timeout_handlers_map_.end())
          hres = handler->second(this->last_message_type_);
        else
          hres = this->default_timeout_handler_(this->last_message_type_);
        if (hres != HandlerError::HANDLER_OK) {
          HAIER_LOGW("Timeout handler error, msg=%02X, err=%d", this->last_message_type_, hres);
        }
      }
      state_ = ProtocolState::IDLE;
      break;
    }
    if (this->transport_.available() > 0)
    {
#if HAIER_LOG_LEVEL > 3
      HAIER_LOGD("Answer delay %dms", std::chrono::duration_cast<std::chrono::milliseconds>(now - last_message_sent_));
#endif
      TimestampedFrame frame;
      this->transport_.pop(frame);
      FrameType msg_type = (FrameType) frame.frame.get_frame_type();
      HandlerError hres;
      std::map<FrameType, AnswerHandler>::const_iterator handler = this->answer_handlers_map_.find(last_message_type_);
      if (handler != this->answer_handlers_map_.end())
        hres = handler->second(this->last_message_type_, msg_type, frame.frame.get_data(), frame.frame.get_data_size());
      else
        hres = this->default_answer_handler_(this->last_message_type_, msg_type, frame.frame.get_data(), frame.frame.get_data_size());
      if (hres != HandlerError::HANDLER_OK)
      {
        HAIER_LOGW("Answer handler error, msg=%02X, answ=%02X, err=%d", this->last_message_type_, msg_type, hres);
      }
      // Answer received, remove message
      this->outgoing_messages_.pop();
      this->retry_time_point_ = now;
      state_ = ProtocolState::IDLE;
    }
    break;
  }
}

bool ProtocolHandler::write_message_(const HaierMessage &message, bool use_crc)
{
  size_t buf_size = message.get_buffer_size();
  bool is_success = true;
  uint8_t frame_type = (uint8_t) message.get_frame_type();
  if (buf_size == 0)
    is_success = this->transport_.send_data(frame_type, nullptr, 0, use_crc) > 0;
  else
  {
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[buf_size]);
    is_success = (message.fill_buffer(buffer.get(), buf_size) > 0) && (this->transport_.send_data(frame_type, buffer.get(), buf_size, use_crc) > 0);
  }
  if (!is_success)
  {
    HAIER_LOGE("Error sending message: %02X", frame_type);
  }
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
#if HAIER_LOG_LEVEL > 3
  last_message_sent_ = now;
#endif
  this->cooldown_time_point_ = now + this->cooldown_interval_;
  return is_success;
}

void ProtocolHandler::set_answer_timeout(long long answer_timeout_miliseconds)
{
  this->set_answer_timeout(std::chrono::milliseconds(answer_timeout_miliseconds));
}

void ProtocolHandler::set_answer_timeout(std::chrono::milliseconds answer_timeout)
{
  this->answer_timeout_interval_ = answer_timeout;
}

void ProtocolHandler::set_cooldown_interval(long long answer_timeout_miliseconds)
{
  this->set_cooldown_interval(std::chrono::milliseconds(answer_timeout_miliseconds));
}

void ProtocolHandler::set_cooldown_interval(std::chrono::milliseconds answer_timeout)
{
  this->cooldown_interval_ = answer_timeout;
}

void ProtocolHandler::send_message(const HaierMessage& message, bool use_crc, uint8_t num_repeats, std::chrono::milliseconds interval)
{
  this->outgoing_messages_.push({ message, use_crc, false, std::min(num_repeats, MAX_PACKET_RETRIES) + 1, interval });
}

void ProtocolHandler::send_message_without_answer(const HaierMessage& message, bool use_crc)
{
  this->outgoing_messages_.push({ message, use_crc, true, 1, std::chrono::milliseconds::zero() });
}

void ProtocolHandler::send_answer(const HaierMessage &answer)
{
  this->send_answer(answer, this->incoming_message_crc_status_);
}

void ProtocolHandler::send_answer(const HaierMessage& answer, bool use_crc)
{
    if (this->processing_message_)
    {
        this->answer_sent_ = this->write_message_(answer, use_crc);
    }
    else
    {
        HAIER_LOGE("Answer can be send only from message handler!");
    }
}

void ProtocolHandler::no_answer()
{
  if (this->processing_message_)
    this->answer_sent_ = true;
}


void ProtocolHandler::set_message_handler(FrameType message_type, MessageHandler handler)
{
  this->message_handlers_map_[message_type] = handler;
}

void ProtocolHandler::remove_message_handler(FrameType message_type)
{
  std::map<FrameType, MessageHandler>::const_iterator it = this->message_handlers_map_.find(message_type);
  if (it != this->message_handlers_map_.end())
    this->message_handlers_map_.erase(it);
}

/// <summary>
/// No way to remove default handler but it is possible to replace it with empty function.
/// </summary>
void ProtocolHandler::set_default_message_handler(MessageHandler handler)
{
  this->default_message_handler_ = handler;
}

void ProtocolHandler::set_answer_handler(FrameType message_type, AnswerHandler handler)
{
  this->answer_handlers_map_[message_type] = handler;
}

void ProtocolHandler::remove_answer_handler(FrameType message_type)
{
  std::map<FrameType, AnswerHandler>::const_iterator it = this->answer_handlers_map_.find(message_type);
  if (it != this->answer_handlers_map_.end())
    this->answer_handlers_map_.erase(it);
}

void ProtocolHandler::set_default_answer_handler(AnswerHandler handler)
{
  this->default_answer_handler_ = handler;
}

void ProtocolHandler::set_timeout_handler(FrameType message_type, TimeoutHandler handler)
{
  this->timeout_handlers_map_[message_type] = handler;
}

void ProtocolHandler::remove_timeout_handler(FrameType message_type)
{
  std::map<FrameType, TimeoutHandler>::const_iterator it = this->timeout_handlers_map_.find(message_type);
  if (it != this->timeout_handlers_map_.end())
    this->timeout_handlers_map_.erase(it);
}

void ProtocolHandler::set_default_timeout_handler(TimeoutHandler handler)
{
  this->default_timeout_handler_ = handler;
}

/// <summary>
/// Default message handler, log everything and return UNSUPPORTED_MESSAGE
/// </summary>
/// <param name="message_type">Type of incoming message</param>
/// <param name="data">Incoming message data</param>
/// <param name="data_size">Size of incoming data</param>
/// <returns>Error code</returns>
HandlerError default_message_handler(FrameType message_type, const uint8_t *data, size_t data_size)
{
  HAIER_LOGW("Unsupported message received: type %02X data: %s", message_type, data_size > 0 ? buf_to_hex(data, data_size).c_str() : "<empty>");
  return HandlerError::UNSUPPORTED_MESSAGE;
}

/// <summary>
/// Default message handler, log everything and return UNSUPPORTED_MESSAGE
/// </summary>
/// <param name="requestType">Request that caused this answer</param>
/// <param name="message_type">Type of incoming message</param>
/// <param name="data">Incoming message data</param>
/// <param name="data_size">Size of incoming data</param>
/// <returns>Error code</returns>
HandlerError default_answer_handler(FrameType requestType, FrameType message_type, const uint8_t *data, size_t data_size)
{
  HAIER_LOGW("Unsupported answer to %02X received: type %02X data: %s", requestType, message_type, data_size > 0 ? buf_to_hex(data, data_size).c_str() : "<empty>");
  return HandlerError::UNSUPPORTED_MESSAGE;
}

/// <summary>
/// Default message handler, log everything and return UNSUPPORTED_MESSAGE
/// </summary>
/// <param name="requestType">Request that caused timeout</param>
/// <returns>Error code</returns>
HandlerError default_timeout_handler(FrameType message_type)
{
  HAIER_LOGW("Message %02X answer timeout", message_type);
  return HandlerError::HANDLER_OK;
}

} // haier_protocol