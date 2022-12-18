#ifndef PROTOCOL_TRANSPORT_H
#define PROTOCOL_TRANSPORT_H
#include <stdint.h>
#include <chrono>
#include <queue>
#include "Utils/haier_log.h"
#include "Utils/circular_buffer.h"
#include "Utils/protocol_stream.h"
#include "Transport/haier_frame.h"

namespace HaierProtocol
{

struct TimestampedFrame
{
    HaierFrame frame;
    std::chrono::steady_clock::time_point timestamp;
};

// Not thread safe!
class TransportLevelHandler
{
public:
    TransportLevelHandler(const TransportLevelHandler&) = delete;
    TransportLevelHandler& operator=(const TransportLevelHandler&) = delete;
    TransportLevelHandler(TransportLevelHandler&&) noexcept;
    explicit TransportLevelHandler(ProtocolStream& stream, size_t bufferSize = MAX_FRAME_SIZE + 0x10) noexcept;
    uint8_t sendData(uint8_t frameType, const uint8_t* data, size_t dataSize, bool useCrc=true);
    size_t readData();
    void processData();
    size_t available() const noexcept { return mIncommingQueue.size(); };
    bool pop(TimestampedFrame& tframe);
    void drop(size_t framesCount);
    void resetProtocol() noexcept;
    virtual ~TransportLevelHandler();
protected:
    void clear();
    void dropBytes(size_t size);
private:
    ProtocolStream&                 mStream;
    CircularBuffer<uint8_t>         mBuffer;
    size_t                          mPos;
    size_t                          mSepCount;
    bool                            mFrameStartFound;
    HaierFrame                      mCurrentFrame;
    std::chrono::steady_clock::time_point   mFrameStart;
    std::queue<TimestampedFrame>    mIncommingQueue;
};

} // HaierProtocol
#endif // PROTOCOL_TRANSPORT_H