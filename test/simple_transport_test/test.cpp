#include <stdint.h>
#include <iostream>
#include <cassert>
#include "utils/circular_buffer.h"
#include "utils/protocol_stream.h"
#include "utils/haier_log.h"
#include "utils/circular_buffer.h"
#include "transport/haier_frame.h"
#include "transport/protocol_transport.h"
#include "console_log.h"
#include "test_macro.h"

class TestStream : public haier_protocol::ProtocolStream
{
public:
    TestStream() : mBuffer(2000) {};
    virtual size_t		available() noexcept { return mBuffer.get_size(); };
    virtual size_t		read_array(uint8_t* data, size_t len) noexcept;
    virtual void		write_array(const uint8_t* data, size_t len) noexcept;
    void addByte(uint8_t val);
    void addBuffer(uint8_t* buf, size_t size);
    size_t get_bytes_counter() const { return mBytesCounter; };
private:
    size_t mBytesCounter{ 0 };
    CircularBuffer<uint8_t>     mBuffer;
};

size_t TestStream::read_array(uint8_t* data, size_t len) noexcept
{
    size_t toRead = mBuffer.get_size();
    if (len < toRead)
        toRead = len;
    return mBuffer.pop(data, toRead);
}

void TestStream::write_array(const uint8_t* data, size_t len) noexcept
{
    mBytesCounter += len;
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
    haier_protocol::TransportLevelHandler transport(stream, 0);
    HAIER_LOGI("Haier protocol transport tests");
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST1)
    {
      TEST_START(1);
      const auto buffer_size = transport.get_buffer_size();
      HAIER_LOGI("Buffer size %u", buffer_size);
      if (buffer_size < CircularBuffer<uint8_t>::CIRCULAR_BUFFER_MINIMUM_SIZE)
          HAIER_LOGE("ERROR: Buffer is too small!");
      TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST2)
    {
        TEST_START(2);
        haier_protocol::TimestampedFrame tsframe;
        // No 0xFF 0xFF at the packet beginning, should drop whole frame
        uint8_t buffer1[] = { 0x0E, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x05, 0xFF, 0x55, 0xFF, 0x55, 0x08, 0xFF, 0x55, 0xD0, 0x8E, 0xFF};
        stream.addBuffer(buffer1, sizeof(buffer1));
        uint8_t buffer2[] = { 0xFF, 0xFF, 0x2A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x02, 0x06, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x00, 0x44, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0xD1, 0x7B};
        stream.addBuffer(buffer2, sizeof(buffer2));
        transport.read_data();
        transport.process_data();
        transport.pop(tsframe);
        transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST3)
    {
        TEST_START(3);
        haier_protocol::TimestampedFrame tsframe;
        uint8_t buffer[] = { 0xFF, 0xFF, 0x0E, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x54, 0x05, 0xFF, 0x55, 0xFF, 0x55, 0x08, 0xFF, 0x55, 0xD0, 0x8E};
        stream.addBuffer(buffer, sizeof(buffer));
        transport.read_data();
        transport.process_data();
        transport.pop(tsframe);
        TEST_END(1, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST4)
    {
        TEST_START(4);
        haier_protocol::TimestampedFrame tsframe;
        uint8_t buffer[] = { 0xFF, 0xFF, 0x2A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x02, 0x06, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x46, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xFF, 0x55, 0xD2 };
        stream.addBuffer(buffer, sizeof(buffer));
        transport.read_data();
        transport.process_data();
        transport.pop(tsframe);
        transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST5)
    {
        TEST_START(5);
        haier_protocol::TimestampedFrame tsframe;
        uint8_t buffer[] = { 0xFF, 0xFF, 0x0E, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x05, 0xFF, 0x55, 0xFF, 0x55, 0x08, 0xFF, 0x55, 0xD0, 0x8E };
        stream.addBuffer(buffer, sizeof(buffer));
        transport.read_data();
        transport.process_data();
        transport.pop(tsframe);
        transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST6)
    {
        TEST_START(6);
        haier_protocol::TimestampedFrame tsframe;
        uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0xFF, 0x55, 0xBB, 0xFF, 0x55, 0xFF, 0x55, 0xD1, 0x3C };
        stream.addBuffer(buffer, sizeof(buffer));
        transport.read_data();
        transport.process_data();
        transport.pop(tsframe);
        transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST7)
    {
        TEST_START(7);
        haier_protocol::TimestampedFrame tsframe;
        uint8_t buffer[] = { 0xFF, 0xFF, 0x2A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x02, 0x06, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x46, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xFF, 0x55, 0xD2 };
        stream.addBuffer(buffer, sizeof(buffer));
        transport.read_data();
        transport.process_data();
        transport.pop(tsframe);
        transport.send_data(tsframe.frame.get_frame_type(), tsframe.frame.get_data(), tsframe.frame.get_data_size(), tsframe.frame.get_use_crc());
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST8)
    {
        TEST_START(8);
        uint8_t buffer[] = { 0x4D, 0x01, 0x02, 0x06, 0xFF, 0x00, 0x56, 0x00, 0xFF, 0x04 };
        const auto old_size = stream.get_bytes_counter();
        transport.send_data(0x02, buffer, sizeof(buffer));
        auto buf_size = stream.get_bytes_counter() - old_size;
        if (buf_size != 2 + haier_protocol::FRAME_HEADER_SIZE + sizeof(buffer) + 1 + 2)
            HAIER_LOGE("Buffers don't match!");
        TEST_END(0, 0);
    }
#endif
    HAIER_LOGI("All tests successfully finished!");
}