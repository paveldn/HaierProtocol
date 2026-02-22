#include <memory>
#include <iomanip>
#include <sstream>
#include "transport/protocol_transport.h"

namespace haier_protocol
{

constexpr std::chrono::duration<long long, std::milli> FRAME_TIMEOUT(300);

TransportLevelHandler::TransportLevelHandler(ProtocolStream &stream, size_t buffer_size) noexcept : stream_(stream),
  buffer_(buffer_size),
  pos_(0),
  sep_count_(0),
  frame_start_found_(false),
  current_frame_()
{
}

uint8_t TransportLevelHandler::send_data(uint8_t frame_type, const uint8_t *data, size_t data_size, bool use_crc)
{
  if (data_size > MAX_FRAME_SIZE - PURE_HEADER_SIZE)
    return 0;
#if (HAIER_LOG_LEVEL > 3)
  static char _header[]{"Sending frame: type 00, data:"};
  const char *_p = hex_map + (frame_type * 2);
  _header[20] = _p[0];
  _header[21] = _p[1];
  HAIER_BUFD(_header, data, data_size);
#endif
  HaierFrame frame = HaierFrame(frame_type, data, (uint8_t)data_size, use_crc);
  size_t size = frame.get_buffer_size();
  std::unique_ptr<uint8_t[]> tmp_buf(new uint8_t[size]);
  frame.fill_buffer(tmp_buf.get(), size);
  HAIER_BUFV("Sending data:", tmp_buf.get(), size);
  this->stream_.write_array(tmp_buf.get(), size);
  return (uint8_t)size;
}

size_t TransportLevelHandler::read_data()
{
  size_t count = this->stream_.available();
  if (count >= this->buffer_.get_capacity())
    count = this->buffer_.get_capacity();
  // Need to make space
  size_t available = this->buffer_.get_space();
  if (count > available)
  {
    this->drop_bytes_(count - available);
    if (this->frame_start_found_)
    {
      // Resetting frame because we will lose it start
      HAIER_LOGW("Frame lost because of buffer overflow");
      this->pos_ = 0;
      this->sep_count_ = 0;
      this->frame_start_found_ = false;
      this->current_frame_.reset();
    }
  }
  size_t size1 = count;
  size_t size2 = 0;
  uint8_t *buf1 = this->buffer_.reserve(size1);
  uint8_t *buf2 = nullptr;
  size1 = this->stream_.read_array(buf1, size1);
  if (count > size1)
  {
    size2 = count - size1;
    buf2 = this->buffer_.reserve(size2);
    size2 = this->stream_.read_array(buf2, size2);
  }
#if (HAIER_LOG_LEVEL > 4)
  if (size1 + size2 > 0)
  {
    log_haier_buffers(haier_protocol::HaierLogLevel::LEVEL_VERBOSE, "Received data:", buf1, size1, buf2, size2);
  }
#endif
  return size1 + size2;
}

void TransportLevelHandler::process_data()
{
  if (this->current_frame_.get_status() > FrameStatus::FRAME_EMPTY)
  {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - this->frame_start_) > FRAME_TIMEOUT)
    {
      // Timeout
      HAIER_LOGW("Frame timeout!");
      this->drop_bytes_(this->pos_);
      this->pos_ = 0;
      this->sep_count_ = 0;
      this->frame_start_found_ = false;
      this->current_frame_.reset();
    }
  }
  if (!this->buffer_.empty())
  {
    size_t buf_size = this->buffer_.get_size();
    int bytes_to_drop = 0;
    while (this->pos_ < buf_size)
    {
      // Searching for the begging of packet
      if (!this->frame_start_found_)
      {
        if (this->buffer_[pos_] == SEPARATOR_BYTE)
        {
          if (this->pos_ + 1 - bytes_to_drop > FRAME_SEPARATORS_COUNT)
            bytes_to_drop = this->pos_ + 1 - FRAME_SEPARATORS_COUNT;
        }
        else
        {
          if (this->pos_ - bytes_to_drop == FRAME_SEPARATORS_COUNT)
          {
            this->frame_start_found_ = true;
            if (bytes_to_drop > 0)
            {
              // Dropping garbage
              this->drop_bytes_(bytes_to_drop);
              buf_size -= bytes_to_drop;
              this->pos_ -= bytes_to_drop;
              bytes_to_drop = 0;
            }
          }
          else
            bytes_to_drop = this->pos_ + 1;
        }
      }
      else
      {
        if (buffer_[pos_] == SEPARATOR_BYTE)
          ++sep_count_;
        switch (this->current_frame_.get_status())
        {
        case FrameStatus::FRAME_EMPTY:
        {
          if (pos_ + 1 - sep_count_ == FRAME_HEADER_SIZE)
          {
            std::unique_ptr<uint8_t[]> headerBuffer(new uint8_t[FRAME_HEADER_SIZE]);
            size_t hPos = 0;
            size_t bPos = 0;
            for (; bPos < FRAME_SEPARATORS_COUNT; ++bPos)
              headerBuffer.get()[hPos++] = buffer_[bPos];
            headerBuffer.get()[hPos++] = buffer_[bPos++];
            bool correctFrame = true;
            while (bPos <= pos_)
            {
              if (buffer_[bPos - 1] != SEPARATOR_BYTE)
                headerBuffer.get()[hPos++] = buffer_[bPos];
              else if (buffer_[bPos] != SEPARATOR_POST_BYTE)
              {
                correctFrame = false;
                break;
              };
              ++bPos;
            }
            if (!correctFrame)
            {
              HAIER_LOGW("Frame parsing error: %d", FrameError::WRONG_POST_SEPARATOR_BYTE);
              buffer_.drop(bPos - 1);
              this->current_frame_.reset();
              pos_ = 0;
              sep_count_ = 0;
              buf_size = buffer_.get_size();
              frame_start_found_ = false;
              continue;
            }
            buffer_.drop(bPos);
            buf_size -= pos_ + 1;
            pos_ = 0;
            sep_count_ = 0;
            FrameError err;
            this->current_frame_.parse_buffer(headerBuffer.get(), hPos, err);
            if (err != FrameError::HEADER_ONLY)
            {
              HAIER_LOGW("Frame parsing error: %d", err);
              this->current_frame_.reset();
              this->frame_start_found_ = false;
            }
            continue;
          }
        }
        break;
        case FrameStatus::FRAME_HEADER_ONLY:
          if (this->pos_ + 1 - this->sep_count_ == this->current_frame_.get_data_size() + (this->current_frame_.get_use_crc() ? 3 : 1))
          {
            std::unique_ptr<uint8_t[]> tmp_buf(new uint8_t[pos_ + 1 - sep_count_]);
            size_t hPos = 0;
            size_t bPos = 0;
            bool correctFrame = true;
            while (bPos <= this->pos_)
            {
              if ((bPos == 0) || (this->buffer_[bPos - 1] != SEPARATOR_BYTE))
                tmp_buf.get()[hPos++] = this->buffer_[bPos];
              else if (this->buffer_[bPos] != SEPARATOR_POST_BYTE)
              {
                correctFrame = false;
                break;
              }
              ++bPos;
            }
            if (!correctFrame)
            {
              HAIER_LOGW("Frame parsing error: %d", FrameError::WRONG_POST_SEPARATOR_BYTE);
              this->buffer_.drop(bPos - 1);
              this->current_frame_.reset();
              this->pos_ = 0;
              this->sep_count_ = 0;
              buf_size = buffer_.get_size();
              this->frame_start_found_ = false;
              continue;
            }
            this->buffer_.drop(bPos);
            FrameError err;
            this->current_frame_.parse_buffer(tmp_buf.get(), hPos, err);
            if (err == FrameError::COMPLETE_FRAME)
            {
#if (HAIER_LOG_LEVEL > 3)
              static char _header[]{"Frame found: type 00, data:"};
              uint8_t _frameType = this->current_frame_.get_frame_type();
              const char *_p = hex_map + (_frameType * 2);
              _header[18] = _p[0];
              _header[19] = _p[1];
              HAIER_BUFD(_header, tmp_buf.get(), this->current_frame_.get_data_size());
#endif
              this->incoming_queue_.push(TimestampedFrame{std::move(this->current_frame_), frame_start_});
            }
            else
            {
              HAIER_LOGW("Frame parsing error: %d", err);
            }
            this->current_frame_.reset();
            this->pos_ = 0;
            this->sep_count_ = 0;
            buf_size = this->buffer_.get_size();
            this->frame_start_found_ = false;
            continue;
          }
          break;
          // No need for separate FrameStatus::fsComplete
        default:
          // Shouldn't get here!
          this->current_frame_.reset();
          this->frame_start_found_ = false;
          continue;
        }
      }
      this->pos_++;
    }
    if (bytes_to_drop > 0)
    {
      drop_bytes_(bytes_to_drop);
      this->pos_ -= bytes_to_drop;
      bytes_to_drop = 0;
    }
  }
}

void TransportLevelHandler::reset_protocol() noexcept
{
  this->current_frame_.reset();
  size_t bytesToDrop = this->buffer_.get_size();
  if (pos_ < bytesToDrop)
    bytesToDrop = pos_;
  this->drop_bytes_(bytesToDrop);
  this->pos_ = 0;
  this->sep_count_ = 0;
  this->frame_start_found_ = false;
}

bool TransportLevelHandler::pop(TimestampedFrame &tframe)
{
  if (this->incoming_queue_.empty())
    return false;
  tframe = std::move(this->incoming_queue_.front());
  this->incoming_queue_.pop();
  return true;
}

void TransportLevelHandler::drop(size_t frames_count)
{
  size_t sz = this->incoming_queue_.size();
  if (frames_count < sz)
    sz = frames_count;
  while (sz-- > 0)
    this->incoming_queue_.pop();
}

TransportLevelHandler::~TransportLevelHandler()
{
}

void TransportLevelHandler::clear_()
{
  HAIER_LOGV("Clearing buffer, data size: %d", this->buffer_.get_size());
  this->buffer_.clear();
  this->pos_ = 0;
  this->sep_count_ = 0;
  this->frame_start_found_ = false;
  this->current_frame_.reset();
}

void TransportLevelHandler::drop_bytes_(size_t size)
{
  this->buffer_.drop(size);
  HAIER_LOGV("Dropping %d bytes", size);
}

} // haier_protocol