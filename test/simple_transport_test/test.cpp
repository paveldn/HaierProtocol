#include <stdint.h>
#include <iostream>
#include "utils/circular_buffer.h"
#include "utils/protocol_stream.h"
#include "utils/haier_log.h"
#include "transport/protocol_transport.h"
#include "console_log.h"

class TestStream : public haier_protocol::ProtocolStream
{
public:
    TestStream() : mBuffer(2000) {};
    virtual size_t		available() noexcept { return mBuffer.get_available(); };
    virtual size_t		read_array(uint8_t* data, size_t len) noexcept;
    virtual void		write_array(const uint8_t* data, size_t len) noexcept;
    void addByte(uint8_t val);
    void addBuffer(uint8_t* buf, size_t size);
private:
    CircularBuffer<uint8_t>     mBuffer;
};

size_t TestStream::read_array(uint8_t* data, size_t len) noexcept
{
    size_t toRead = mBuffer.get_available();
    if (len < toRead)
        toRead = len;
    return mBuffer.pop(data, toRead);
}

void TestStream::write_array(const uint8_t* data, size_t len) noexcept
{
    console_logger(haier_protocol::HaierLogLevel::LEVEL_INFO, "Stream", buf_to_hex(data, len).c_str());
}

void TestStream::addByte(uint8_t val)
{
    mBuffer.push(val);
}

void TestStream::addBuffer(uint8_t* buf, size_t size)
{
    mBuffer.push(buf, size);
}

int main()
{
    haier_protocol::set_log_handler(console_logger);
    TestStream stream;
    haier_protocol::TimestampedFrame tsframe;
    haier_protocol::TransportLevelHandler transport(stream);
#if 1
    {
        HAIER_LOGI("Test #1");
        // No 0xFF 0xFF at the packet beginning, should drop whole frame
        uint8_t buffer[] = { 0x0E, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x05, 0xFF, 0x55, 0xFF, 0x55, 0x08, 0xFF, 0x55, 0xD0, 0x8E, 0xFF};
        stream.addBuffer(buffer, sizeof(buffer));
    }
#endif
#if 1
    {
        HAIER_LOGI("Test #2");
        uint8_t buffer[] = { 0xFF, 0xFF, 0x2A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x02, 0x06, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x00, 0x44, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0xD1, 0x7B};
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.read_data();
    transport.process_data();
    transport.pop(tsframe);
    transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
#endif
#if 1
    {
        HAIER_LOGI("Test #3");
        uint8_t buffer[] = { 0xFF, 0xFF, 0x0E, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x54, 0x05, 0xFF, 0x55, 0xFF, 0x55, 0x08, 0xFF, 0x55, 0xD0, 0x8E};
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.read_data();
    transport.process_data();
    transport.pop(tsframe);
#endif
#if 1
    {
        HAIER_LOGI("Test #4");
        uint8_t buffer[] = { 0xFF, 0xFF, 0x2A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x02, 0x06, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x46, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xFF, 0x55, 0xD2 };
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.read_data();
    transport.process_data();
    transport.pop(tsframe);
    transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
#endif
#if 1
    {
        HAIER_LOGI("Test #5");
        uint8_t buffer[] = { 0xFF, 0xFF, 0x0E, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x05, 0xFF, 0x55, 0xFF, 0x55, 0x08, 0xFF, 0x55, 0xD0, 0x8E };
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.read_data();
    transport.process_data();
    transport.pop(tsframe);
    transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
#endif
#if 1
    {
        HAIER_LOGI("Test #6");
        uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0xFF, 0x55, 0xBB, 0xFF, 0x55, 0xFF, 0x55, 0xD1, 0x3C };
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.read_data();
    transport.process_data();
    transport.pop(tsframe);
    transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
#endif
#if 1
    {
        HAIER_LOGI("Test #7");
        uint8_t buffer[] = { 0xFF, 0xFF, 0x2A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x02, 0x06, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x46, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xFF, 0x55, 0xD2 };
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.read_data();
    transport.process_data();
    transport.pop(tsframe);
    transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
#endif
#if 1
    {
        HAIER_LOGI("Test #8");
        uint8_t buffer[] = { 0x4D, 0x01, 0x02, 0x06, 0xFF, 0x00, 0x56, 0x00, 0xFF, 0x04 };
        transport.send_data(0x02, buffer, sizeof(buffer));
    }
#endif
    HAIER_LOGI("Test finished");
    // Should be "Warnings: 1, errors: 0"
    unsigned int warn = get_warnings_count();
	unsigned int  errors = get_errors_count();
	std::cout << "Test results, warning: " << warn << " errors: " << errors << std::endl;
	if ((warn != 1) || (errors != 0))
		exit(1);
}