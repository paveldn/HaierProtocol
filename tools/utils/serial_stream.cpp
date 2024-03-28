#include "serial_stream.h"
#include <iostream>

#if __linux__
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

SerialStream::SerialStream(const std::string& port_path) : buffer_(SERIAL_BUFFER_SIZE) {
#if _WIN32
    constexpr char win_prefix[] = "\\\\.\\";
    std::string port_win = port_path;
    if (port_win.rfind(win_prefix, 0) != 0)
        port_win = std::string(win_prefix).append(port_win);
    handle_ = CreateFile(port_win.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (is_valid()) {
        DCB serialParams = { 0 };
        serialParams.DCBlength = sizeof(serialParams);
        GetCommState(handle_, &serialParams);
        serialParams.BaudRate = 9600;
        serialParams.ByteSize = 8;
        serialParams.fBinary = 1;
        serialParams.fRtsControl = 0;
        serialParams.fDtrControl = 0;
        serialParams.XonLim = 2048;
        serialParams.XoffLim = 512;
        serialParams.Parity = 0;
        serialParams.XonChar = 17;
        serialParams.XoffChar = 19;
        serialParams.EofChar = 26;
        SetCommState(handle_, &serialParams);
        COMMTIMEOUTS timeout = { 0 };
        timeout.ReadIntervalTimeout = MAXDWORD;
        timeout.ReadTotalTimeoutConstant = 0;
        timeout.ReadTotalTimeoutMultiplier = 0;
        timeout.WriteTotalTimeoutConstant = 2;
        timeout.WriteTotalTimeoutMultiplier = 0;
        SetCommTimeouts(handle_, &timeout);
    }
#else
    handle_ = open(port_path.c_str(), O_RDWR | O_NOCTTY );
    if (is_valid()) {
      struct termios tty;
      if (tcgetattr(handle_, &tty) != 0) {
        handle_ = -1;
        return;
      }
      tty.c_cflag &= ~PARENB;
      tty.c_cflag &= ~CSTOPB;
      tty.c_cflag &= ~CSIZE;
      tty.c_cflag |= CS8;
      tty.c_cflag &= ~CRTSCTS;
      tty.c_cflag |= CREAD | CLOCAL;
      tty.c_lflag &= ~ICANON;
      tty.c_lflag &= ~ECHO;
      tty.c_lflag &= ~ECHOE;
      tty.c_lflag &= ~ECHONL;
      tty.c_lflag &= ~ISIG;
      tty.c_iflag &= ~(IXON | IXOFF | IXANY);
      tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
      tty.c_oflag &= ~OPOST;
      tty.c_oflag &= ~ONLCR;
      tty.c_cc[VTIME] = 0;
      tty.c_cc[VMIN] = 0;
      cfsetispeed(&tty, B9600);
      cfsetospeed(&tty, B9600);
      if (tcsetattr(handle_, TCSANOW, &tty) != 0) {
        handle_ = -1;
        return;
      }
    }
#endif
}

SerialStream::~SerialStream() {
  if (is_valid()) {
#if _WIN32
    CloseHandle(handle_);
    handle_ = INVALID_HANDLE_VALUE;
#else
    close(handle_);
    handle_ = -1;
#endif
  }
}

bool SerialStream::is_valid() const {
#if _WIN32
  return handle_ != INVALID_HANDLE_VALUE;
#else
  return handle_ >= 0;
#endif
};

size_t SerialStream::available() noexcept {
    if (!is_valid())
        return 0;
    if (buffer_.empty())
    {
        static uint8_t tmp_buf[SERIAL_BUFFER_SIZE];
        unsigned long size;
        int status;
#if _WIN32
        status = ReadFile(handle_, tmp_buf, SERIAL_BUFFER_SIZE, &size, nullptr);
#else
        status = read(handle_, tmp_buf, SERIAL_BUFFER_SIZE);
        if (status >= 0) {
            size = status;
            status = 1;
        }
        else {
            status = 0;
            size = 0;
        }
#endif
        if ((status != 0) && (size > 0))
            buffer_.push(tmp_buf, size);
        else
            return 0;
    }
    return buffer_.get_size();
};
size_t SerialStream::read_array(uint8_t* data, size_t len) noexcept {
    if (!is_valid() || buffer_.empty())
        return 0;
    size_t av = buffer_.get_size();
    if (av < len)
        len = av;
    return buffer_.pop(data, len);
}
void SerialStream::write_array(const uint8_t* data, size_t len) noexcept {
    if (!is_valid())
        return;
#if _WIN32
    WriteFile(handle_, data, len, nullptr, nullptr);
#else
    write(handle_, data, len);
#endif
}

