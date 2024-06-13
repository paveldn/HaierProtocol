#ifndef VIRTUAL_STREAM
#define VIRTUAL_STREAM
#include <stdint.h>
#include <cstddef>
#include "utils/protocol_stream.h"
#include "utils/circular_buffer.h"

enum class StreamDirection{
    DIRECTION_A,
    DIRECTION_B
};
class VirtualStreamHolder;

class VirtualStream : public haier_protocol::ProtocolStream {
public:
    VirtualStream() = delete;
    VirtualStream& operator=(const VirtualStream&) = delete;
    size_t available() noexcept override;
    size_t read_array(uint8_t* data, size_t len) noexcept override;
    void write_array(const uint8_t* data, size_t len) noexcept override;
protected:
    friend class VirtualStreamHolder;
    VirtualStream(CircularBuffer<uint8_t>& tx_buffer, CircularBuffer<uint8_t>& rx_buffer);
private:
    CircularBuffer<uint8_t>& tx_buffer_;
    CircularBuffer<uint8_t>& rx_buffer_;
};

class VirtualStreamHolder {
public:
    VirtualStreamHolder();
    VirtualStream& get_stream_reference(StreamDirection);
private:
    CircularBuffer<uint8_t> buffers_[2];
    VirtualStream streams_[2];
};

#endif // VIRTUAL_STREAM