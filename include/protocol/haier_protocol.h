#ifndef HAIER_PROTOCOL_H
#define HAIER_PROTOCOL_H

#include <cstdint>
#include <chrono>
#include <functional>
#include <map>
#include <queue>
#include "transport/protocol_transport.h"
#include "protocol/haier_message.h"

namespace haier_protocol
{

enum class HandlerError
{
    HANDLER_OK = 0,
    UNSUPPORTED_MESSAGE,
    UNEXPECTED_MESSAGE,
    UNSUPPORTED_SUBCOMMAND,
    WRONG_MESSAGE_STRUCTURE,
    RUNTIME_ERROR,
    UNKNOWN_ERROR,
    INVALID_ANSWER,
};

// Message handler type. Expected that function sends answer back.
// argument 1: Incoming message type
// argument 2: Incoming data buffer (nullptr if none)
// argument 3: Incoming data buffer size
// return: Result of processing
using MessageHandler = std::function<HandlerError(FrameType, const uint8_t*, size_t)>;

// Answers handler type.
// argument 1: Request message type that caused this answer
// argument 2: Incoming message type
// argument 3: Incoming data buffer (nullptr if none)
// argument 4: Incoming data buffer size
// return: Result of processing
using AnswerHandler = std::function<HandlerError(FrameType, FrameType, const uint8_t*, size_t)>;

// Timeout handler type.
// argument 1: Request message type that caused this answer
// return: Result of processing
using TimeoutHandler = std::function<HandlerError(FrameType)>;

HandlerError default_message_handler(FrameType message_type, const uint8_t* data, size_t data_size);
HandlerError default_answer_handler(FrameType message_type, FrameType request_type, const uint8_t* data, size_t data_size);
HandlerError default_timeout_handler(FrameType message_type);

class ProtocolHandler 
{
public:
    ProtocolHandler() = delete;
    ProtocolHandler(const ProtocolHandler&) = delete;
    ProtocolHandler& operator=(const ProtocolHandler&) = delete;
    explicit ProtocolHandler(ProtocolStream&) noexcept;
    size_t get_outgoing_queue_size() const noexcept {return this->outgoing_messages_.size(); };
    void set_answer_timeout(long long answer_timeout_miliseconds);
    void set_answer_timeout(std::chrono::milliseconds answer_timeout);
    void set_cooldown_interval(long long answer_timeout_miliseconds);
    void set_cooldown_interval(std::chrono::milliseconds answer_timeout);
    void send_message(const HaierMessage& message, bool use_crc, uint8_t num_retries = 0, std::chrono::milliseconds interval = std::chrono::milliseconds::zero());
    void send_answer(const HaierMessage& answer);
    void send_answer(const HaierMessage& answer, bool use_crc);
    void set_message_handler(FrameType message_type, MessageHandler handler);
    void remove_message_handler(FrameType message_type);
    void set_default_message_handler(MessageHandler handler);
    void set_answer_handler(FrameType message_type, AnswerHandler handler);
    void remove_answer_handler(FrameType message_type);
    void set_default_answer_handler(AnswerHandler handler);
    void set_timeout_handler(FrameType message_type, TimeoutHandler handler);
    void remove_timeout_handler(FrameType message_type);
    void set_default_timeout_handler(TimeoutHandler handler);
    virtual void loop();
protected:
    bool write_message_(const HaierMessage& message, bool use_crc);
    enum class ProtocolState
    {
        IDLE,
        WAITING_FOR_ANSWER,
    };
    struct OutgoingQueueItem
    {
        const HaierMessage message;
        bool use_crc;
        int number_of_retries;
        std::chrono::milliseconds retry_interval;
    };
    using OutgoingQueue = std::queue<OutgoingQueueItem>;
    TransportLevelHandler                   transport_;
    std::map<FrameType, MessageHandler>     message_handlers_map_;
    std::map<FrameType, AnswerHandler>      answer_handlers_map_;
    std::map<FrameType, TimeoutHandler>     timeout_handlers_map_;
    OutgoingQueue                           outgoing_messages_;
    MessageHandler                          default_message_handler_;
    AnswerHandler                           default_answer_handler_;
    TimeoutHandler                          default_timeout_handler_;
    ProtocolState                           state_;
    bool                                    processing_message_;
    bool                                    incoming_message_crc_status_;
    bool                                    answer_sent_;
    FrameType                               last_message_type_;
    std::chrono::milliseconds               answer_timeout_interval_;
    std::chrono::milliseconds               cooldown_interval_;
    std::chrono::steady_clock::time_point   cooldown_time_point_;
    std::chrono::steady_clock::time_point   answer_time_point_;
    std::chrono::steady_clock::time_point   retry_time_point_;

};


} // HaierProtocol
#endif // HAIER_PROTOCOL_H