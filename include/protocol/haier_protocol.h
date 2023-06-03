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
using MessageHandler = std::function<HandlerError(uint8_t, const uint8_t*, size_t)>;

// Answers handler type.
// argument 1: Request message type that caused this answer
// argument 2: Incoming message type
// argument 3: Incoming data buffer (nullptr if none)
// argument 4: Incoming data buffer size
// return: Result of processing
using AnswerHandler = std::function<HandlerError(uint8_t, uint8_t, const uint8_t*, size_t)>;

// Timeout handler type.
// argument 1: Request message type that caused this answer
// return: Result of processing
using TimeoutHandler = std::function<HandlerError(uint8_t)>;

HandlerError default_message_handler(uint8_t message_type, const uint8_t* data, size_t data_size);
HandlerError default_answer_handler(uint8_t message_type, uint8_t request_type, const uint8_t* data, size_t data_size);
HandlerError default_timeout_handler(uint8_t message_type);

class ProtocolHandler 
{
public:
    ProtocolHandler() = delete;
    ProtocolHandler(const ProtocolHandler&) = delete;
    ProtocolHandler& operator=(const ProtocolHandler&) = delete;
    explicit ProtocolHandler(ProtocolStream&) noexcept;
    size_t get_outgoing_queue_size() const noexcept {return this->outgoing_messages_.size(); };
    void send_message(const HaierMessage& message, bool use_crc);
    void send_message(const HaierMessage& message, bool use_crc, long long answer_timeout_miliseconds);
    void send_message(const HaierMessage& message, bool use_crc, std::chrono::milliseconds answer_timeout);
    void send_answer(const HaierMessage& answer);
    void send_answer(const HaierMessage& answer, bool use_crc);
    void set_message_handler(uint8_t message_type, MessageHandler handler);
    void remove_message_handler(uint8_t message_type);
    void set_default_message_handler(MessageHandler handler);
    void set_answer_handler(uint8_t message_type, AnswerHandler handler);
    void remove_answer_handler(uint8_t message_type);
    void set_default_answer_handler(AnswerHandler handler);
    void set_timeout_handler(uint8_t message_type, TimeoutHandler handler);
    void remove_timeout_handler(uint8_t message_type);
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
        const std::chrono::milliseconds answer_timeout;
    };
    using OutgoingQueue = std::queue<OutgoingQueueItem>;
    TransportLevelHandler                   transport_;
    std::map<uint8_t, MessageHandler>       message_handlers_map_;
    std::map<uint8_t, AnswerHandler>        answer_handlers_map_;
    std::map<uint8_t, TimeoutHandler>       timeout_handlers_map_;    
    OutgoingQueue                           outgoing_messages_;
    MessageHandler                          default_message_handler_;
    AnswerHandler                           default_answer_handler_;
    TimeoutHandler                          default_timeout_handler_;
    ProtocolState                           state_;
    bool                                    processing_message_;
    bool                                    incoming_message_crc_status_;
    bool                                    answer_sent_;
    uint8_t                                 last_message_type_;
    std::chrono::steady_clock::time_point   cooldown_timeout_;
    std::chrono::steady_clock::time_point   answer_timeout_;
};


} // HaierProtocol
#endif // HAIER_PROTOCOL_H