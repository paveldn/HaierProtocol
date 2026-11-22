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

// Feeds a buffer to the transport, either at once or byte by byte (to simulate data arriving
// slowly from the UART), and returns how many frames were found. Found frames are discarded.
static size_t feed_and_count_frames(TestStream& stream, haier_protocol::TransportLevelHandler& transport,
                                    const uint8_t* buf, size_t size, bool byte_by_byte)
{
    if (byte_by_byte)
    {
        for (size_t i = 0; i < size; i++)
        {
            stream.addByte(buf[i]);
            transport.read_data();
            transport.process_data();
        }
    }
    else
    {
        stream.addBuffer(const_cast<uint8_t*>(buf), size);
        transport.read_data();
        transport.process_data();
    }
    size_t frames = 0;
    haier_protocol::TimestampedFrame tsframe;
    while (transport.pop(tsframe))
        ++frames;
    return frames;
}

// Status answer used as the "next frame" after the frame under test, to make sure the frame
// under test doesn't swallow the beginning of the following one.
static const uint8_t NEXT_FRAME[] = { 0xFF, 0xFF, 0x0A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x99, 0xB3, 0xB4 };

static void check_frames(TestStream& stream, haier_protocol::TransportLevelHandler& transport,
                         const uint8_t* frame, size_t size, size_t expected_frames)
{
    for (int byte_by_byte = 0; byte_by_byte < 2; byte_by_byte++)
    {
        size_t frames = feed_and_count_frames(stream, transport, frame, size, byte_by_byte != 0);
        frames += feed_and_count_frames(stream, transport, NEXT_FRAME, sizeof(NEXT_FRAME), byte_by_byte != 0);
        if (frames != expected_frames + 1)
            HAIER_LOGE("Expected %u frames, got %u (%s)", (unsigned) expected_frames + 1, (unsigned) frames,
                       byte_by_byte ? "byte by byte" : "single chunk");
    }
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
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST9)
    {
        // Real status answer from a Haier AD71S2SS1FA(H) (hOn, E++2.18): CRC is 0xFF49 and the
        // device sends it without the 0x55 post separator byte
        TEST_START(9);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x32, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x06, 0x0A, 0x81, 0x00, 0x02, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x2A, 0x32, 0x56, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0xFF, 0x49 };
        check_frames(stream, transport, buffer, sizeof(buffer), 1);
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST10)
    {
        // CRC high byte 0xFF, escaped
        TEST_START(10);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x02, 0x00, 0x96, 0x34, 0xFF, 0x55, 0x1F };
        check_frames(stream, transport, buffer, sizeof(buffer), 1);
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST11)
    {
        // CRC high byte 0xFF, not escaped
        TEST_START(11);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x02, 0x00, 0x96, 0x34, 0xFF, 0x1F };
        check_frames(stream, transport, buffer, sizeof(buffer), 1);
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST12)
    {
        // CRC low byte 0xFF, escaped
        TEST_START(12);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x02, 0x7C, 0x03, 0x1D, 0x50, 0xFF, 0x55 };
        check_frames(stream, transport, buffer, sizeof(buffer), 1);
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST13)
    {
        // CRC low byte 0xFF, not escaped
        TEST_START(13);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x02, 0x7C, 0x03, 0x1D, 0x50, 0xFF };
        check_frames(stream, transport, buffer, sizeof(buffer), 1);
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST14)
    {
        // CRC 0xFFFF, escaped
        TEST_START(14);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x02, 0x7F, 0xD7, 0xF4, 0xFF, 0x55, 0xFF, 0x55 };
        check_frames(stream, transport, buffer, sizeof(buffer), 1);
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST15)
    {
        // CRC 0xFFFF, not escaped
        TEST_START(15);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x02, 0x7F, 0xD7, 0xF4, 0xFF, 0xFF };
        check_frames(stream, transport, buffer, sizeof(buffer), 1);
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST16)
    {
        // CRC 0xFF55 (ambiguous), escaped
        TEST_START(16);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x02, 0xE7, 0xD6, 0x5B, 0xFF, 0x55, 0x55 };
        check_frames(stream, transport, buffer, sizeof(buffer), 1);
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST17)
    {
        // CRC 0xFF55 (ambiguous), not escaped
        TEST_START(17);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x02, 0xE7, 0xD6, 0x5B, 0xFF, 0x55 };
        check_frames(stream, transport, buffer, sizeof(buffer), 1);
        TEST_END(0, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST18)
    {
        // Wrong CRC must still be reported (once per pass) and must not break the next frame
        TEST_START(18);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0x02, 0x00, 0x96, 0x34, 0x12, 0x34 };
        check_frames(stream, transport, buffer, sizeof(buffer), 0);
        TEST_END(2, 0);
    }
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST19)
    {
        // Unescaped 0xFF in the data is still an error (only CRC is relaxed)
        TEST_START(19);
        const uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0xFF, 0xBB, 0xFF, 0x55, 0xFF, 0x55, 0xD1, 0x3C };
        check_frames(stream, transport, buffer, sizeof(buffer), 0);
        TEST_END(2, 0);
    }
#endif
    HAIER_LOGI("All tests successfully finished!");
}