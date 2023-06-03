#ifndef PROTOCOL_STREAM_H
#define PROTOCOL_STREAM_H

namespace haier_protocol
{

class ProtocolStream
{
public:
    // Return number of bytes available in read buffer
    virtual size_t      available() noexcept = 0;
    // Read min(len, available()) bytes to data, return the number of bytes read
    virtual size_t      read_array(uint8_t* data, size_t len) noexcept = 0;
    // Write len bytes from data
    virtual void        write_array(const uint8_t* data, size_t len) noexcept = 0;
};

}
#endif // PROTOCOL_STREAM_H