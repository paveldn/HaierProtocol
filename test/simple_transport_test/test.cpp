#include <stdint.h>
#include "Utils/circular_buffer.h"
#include "Utils/protocol_stream.h"
#include "Utils/haier_log.h"
#include "Transport/protocol_transport.h"
#include "console_log.h"

class TestStream : public HaierProtocol::ProtocolStream
{
public:
    TestStream() : mBuffer(2000) {};
    virtual size_t		available() noexcept { return mBuffer.getAvailable(); };
    virtual size_t		read_array(uint8_t* data, size_t len) noexcept;
    virtual void		write_array(const uint8_t* data, size_t len) noexcept;
    void addByte(uint8_t val);
    void addBuffer(uint8_t* buf, size_t size);
private:
    CircularBuffer<uint8_t>     mBuffer;
};

size_t TestStream::read_array(uint8_t* data, size_t len) noexcept
{
    size_t toRead = mBuffer.getAvailable();
    if (len < toRead)
        toRead = len;
    return mBuffer.pop(data, toRead);
}

void TestStream::write_array(const uint8_t* data, size_t len) noexcept
{
    console_logger(HaierProtocol::HaierLogLevel::llInfo, "Stream", buf2hex(data, len).c_str());
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
    HaierProtocol::setLogHandler(console_logger);
    TestStream stream;
    HaierProtocol::TimestampedFrame tsframe;
    HaierProtocol::TransportLevelHandler transport(stream);
#if 1
    {
        // No 0xFF 0xFF at the packet beginning, should drop whole frame
        uint8_t buffer[] = { 0x0E, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x05, 0xFF, 0x55, 0xFF, 0x55, 0x08, 0xFF, 0x55, 0xD0, 0x8E, 0xFF};
        stream.addBuffer(buffer, sizeof(buffer));
    }
#endif
#if 1
    {
        uint8_t buffer[] = { 0xFF, 0xFF, 0x2A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x02, 0x06, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x00, 0x44, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0xD1, 0x7B};
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.readData();
    transport.processData();
    transport.pop(tsframe);
    transport.sendData(tsframe.frame.getFrameType(), tsframe.frame.getData(), tsframe.frame.getDataSize(), tsframe.frame.getUseCrc());
#endif
#if 1
    {
        uint8_t buffer[] = { 0xFF, 0xFF, 0x2A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x02, 0x06, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x46, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xFF, 0x55, 0xD2 };
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.readData();
    transport.processData();
    transport.pop(tsframe);
    transport.sendData(tsframe.frame.getFrameType(), tsframe.frame.getData(), tsframe.frame.getDataSize(), tsframe.frame.getUseCrc());
#endif
#if 1
    {
        uint8_t buffer[] = { 0xFF, 0xFF, 0x0E, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0x05, 0xFF, 0x55, 0xFF, 0x55, 0x08, 0xFF, 0x55, 0xD0, 0x8E };
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.readData();
    transport.processData();
    transport.pop(tsframe);
    transport.sendData(tsframe.frame.getFrameType(), tsframe.frame.getData(), tsframe.frame.getDataSize(), tsframe.frame.getUseCrc());
#endif
#if 1
    {
        uint8_t buffer[] = { 0xFF, 0xFF, 0x0D, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4D, 0x01, 0xFF, 0x55, 0xBB, 0xFF, 0x55, 0xFF, 0x55, 0xD1, 0x3C };
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.readData();
    transport.processData();
    transport.pop(tsframe);
    transport.sendData(tsframe.frame.getFrameType(), tsframe.frame.getData(), tsframe.frame.getDataSize(), tsframe.frame.getUseCrc());
#endif
#if 1
    {
        uint8_t buffer[] = { 0xFF, 0xFF, 0x2A, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x6D, 0x01, 0x02, 0x06, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x46, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xFF, 0x55, 0xD2 };
        stream.addBuffer(buffer, sizeof(buffer));
    }
    transport.readData();
    transport.processData();
    transport.pop(tsframe);
    transport.sendData(tsframe.frame.getFrameType(), tsframe.frame.getData(), tsframe.frame.getDataSize(), tsframe.frame.getUseCrc());
#endif
#if 1
    {
        uint8_t buffer[] = { 0x4D, 0x01, 0x02, 0x06, 0xFF, 0x00, 0x56, 0x00, 0xFF, 0x04 };
        transport.sendData(0x02, buffer, sizeof(buffer));
    }
#endif
}