#ifndef SERIAL_STREAM_WIN
#define SERIAL_STREAM_WIN
#include <string>
#include <thread>
#include <windows.h>
#include "utils/protocol_stream.h"
#include "utils/circular_buffer.h"

#ifndef _WIN32
#error This implementation of Haier protocol stream is for Windowws only
#endif

#define SERIAL_BUFFER_SIZE 2048

class SerailStream : public haier_protocol::ProtocolStream
{
public:
    SerailStream() = delete;
    SerailStream(const std::string& port_path);
    ~SerailStream();
    bool is_valid() const { return handle_ != INVALID_HANDLE_VALUE; };
    size_t available() noexcept override;
    size_t read_array(uint8_t* data, size_t len) noexcept override;
    void write_array(const uint8_t* data, size_t len) noexcept override;
private:
    HANDLE handle_{ INVALID_HANDLE_VALUE };
    CircularBuffer<uint8_t> buffer_;
    std::thread read_thread_;
};

#endif // SERIAL_STREAM_WIN