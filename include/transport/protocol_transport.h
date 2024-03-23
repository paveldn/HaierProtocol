#ifndef PROTOCOL_TRANSPORT_H
#define PROTOCOL_TRANSPORT_H
#include <stdint.h>
#include <chrono>
#include <queue>
#include "utils/haier_log.h"
#include "utils/circular_buffer.h"
#include "utils/protocol_stream.h"
#include "transport/haier_frame.h"

namespace haier_protocol
{

struct TimestampedFrame
{
    HaierFrame frame;
    std::chrono::steady_clock::time_point timestamp;
};

class TransportLevelHandler
{
public:
    TransportLevelHandler(const TransportLevelHandler&) = delete;
    TransportLevelHandler& operator=(const TransportLevelHandler&) = delete;
    explicit TransportLevelHandler(ProtocolStream& stream, size_t buffer_size) noexcept;
    uint8_t send_data(uint8_t frameType, const uint8_t* data, size_t data_size, bool use_crc=true);
    size_t read_data();
    void process_data();
    size_t get_buffer_size() noexcept { return this->buffer_.get_capacity(); };
    size_t available() const noexcept { return this->incoming_queue_.size(); };
    bool pop(TimestampedFrame& tframe);
    void drop(size_t frames_count);
    void reset_protocol() noexcept;
    virtual ~TransportLevelHandler();
protected:
    void clear_();
    void drop_bytes_(size_t size);
    ProtocolStream&                 stream_;
    CircularBuffer<uint8_t>         buffer_;
    size_t                          pos_;
    size_t                          sep_count_;
    bool                            frame_start_found_;
    HaierFrame                      current_frame_;
    std::chrono::steady_clock::time_point   frame_start_;
    std::queue<TimestampedFrame>    incoming_queue_;
};

} // HaierProtocol
#endif // PROTOCOL_TRANSPORT_H