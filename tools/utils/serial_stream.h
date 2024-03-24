#ifndef SERIAL_STREAM
#define SERIAL_STREAM
#include <string>
#include <thread>
#include "utils/protocol_stream.h"
#include "utils/circular_buffer.h"

#if _WIN32
#include <windows.h>
#elif !__linux__
#error This implementation of Haier protocol stream is for Windows or Linux only
#endif

#define SERIAL_BUFFER_SIZE 2048

class SerialStream : public haier_protocol::ProtocolStream
{
public:
    SerialStream() = delete;
    SerialStream(const std::string& port_path);
    ~SerialStream();
    bool is_valid() const;
    size_t available() noexcept override;
    size_t read_array(uint8_t* data, size_t len) noexcept override;
    void write_array(const uint8_t* data, size_t len) noexcept override;
private:
#if __linux__
  int handle_{ -1 };
#elif _WIN32
  HANDLE handle_{ INVALID_HANDLE_VALUE };
#endif
    CircularBuffer<uint8_t> buffer_;
    std::thread read_thread_;
};

#endif // SERIAL_STREAM