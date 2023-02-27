#include "serial_stream.h"

SerailStream::SerailStream(const std::string& port_path) : buffer_(SERIAL_BUFFER_SIZE) {
    handle_ = CreateFile(port_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (is_valid()) {
        DCB serialParams = { 0 };
        serialParams.DCBlength = sizeof(serialParams);
        GetCommState(handle_, &serialParams);
        serialParams.BaudRate = 9600;
        serialParams.ByteSize = 8;
        serialParams.StopBits = 1;
        serialParams.Parity = 0;

        SetCommState(handle_, &serialParams);
        COMMTIMEOUTS timeout = { 0 };
        timeout.ReadIntervalTimeout = MAXDWORD;
        timeout.ReadTotalTimeoutConstant = 0;
        timeout.ReadTotalTimeoutMultiplier = 0;
        timeout.WriteTotalTimeoutConstant = 2;
        timeout.WriteTotalTimeoutMultiplier = 0;
        SetCommTimeouts(handle_, &timeout);
    }
}

SerailStream::~SerailStream() {
    if (is_valid())
        CloseHandle(handle_);
}

size_t SerailStream::available() noexcept {
    if (!is_valid())
        return 0;
    if (buffer_.empty())
    {
        static uint8_t tmp_buf[SERIAL_BUFFER_SIZE];
        unsigned long size;
        int status = ReadFile(handle_, tmp_buf, SERIAL_BUFFER_SIZE, &size, nullptr);
        if ((status != 0) && (size > 0))
            buffer_.push(tmp_buf, size);
        else
            return 0;
    }
    return buffer_.get_available();
};
size_t SerailStream::read_array(uint8_t* data, size_t len) noexcept {
    if (!is_valid() || buffer_.empty())
        return 0;
    size_t av = buffer_.get_available();
    if (av < len)
        len = av;
    return buffer_.pop(data, len);
}
void SerailStream::write_array(const uint8_t* data, size_t len) noexcept {
    if (!is_valid())
        return;
    WriteFile(handle_, data, len, nullptr, nullptr);
}

