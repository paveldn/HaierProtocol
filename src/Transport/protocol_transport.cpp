#include <memory>
#include <iomanip>
#include <sstream>
#include "Transport/protocol_transport.h"

#define TAG	"haier.transport"

namespace HaierProtocol
{

constexpr std::chrono::duration<long long, std::milli> FRAME_TIMEOUT(300);

TransportLevelHandler::TransportLevelHandler(TransportLevelHandler&& source) noexcept :
	mStream(source.mStream),
	mBuffer(std::move(source.mBuffer)),
	mPos(source.mPos),
	mSepCount(source.mSepCount),
	mFrameStartFound(source.mFrameStartFound),
	mFrameStart(source.mFrameStart),
	mCurrentFrame(std::move(source.mCurrentFrame)),
	mIncommingQueue(std::move(source.mIncommingQueue))
{
}

TransportLevelHandler::TransportLevelHandler(ProtocolStream& stream, size_t bufferSize) noexcept :
	mStream(stream),
	mBuffer(bufferSize),
	mPos(0),
	mSepCount(0),
	mFrameStartFound(false),
	mCurrentFrame()
{
}

uint8_t TransportLevelHandler::sendData(uint8_t frameType, const uint8_t* data, size_t dataSize, bool useCrc)
{
	if (dataSize > MAX_FRAME_SIZE - PURE_HEADER_SIZE)
		return 0;
 	HAIER_LOGD(TAG, "Sending frame: type %02X, data: %s", frameType, dataSize == 0 ? "<empty>" : buf2hex(data, dataSize).c_str());
	HaierFrame frame = HaierFrame(frameType, data, (uint8_t)dataSize, useCrc);
	size_t size = frame.getBufferSize();
	std::unique_ptr<uint8_t[]> tmpBuf(new uint8_t[size]);
	frame.fillBuffer(tmpBuf.get(), size);
	HAIER_LOGV(TAG, "Sending data: %s", buf2hex(tmpBuf.get(), size).c_str());
	mStream.write_array(tmpBuf.get(), size);
	return (uint8_t)size;
} 

size_t TransportLevelHandler::readData()
{
	size_t bytes_read = 0;
	uint8_t val;
	size_t count = mStream.available();
#if (HAIER_LOG_LEVEL > 4)
	std::stringstream outBuf;
#endif
	while ((bytes_read < count) && (mStream.read_array(&val, 1) > 0))
	{
#if (HAIER_LOG_LEVEL > 4)
		outBuf << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int)val << ' ';
#endif
		// IF there is 0xFF 0x55 replace it with 0xFF
		if (mBuffer.push(val) == 0)
			break;
		bytes_read++;
	}
#if (HAIER_LOG_LEVEL > 4)
	if (bytes_read > 0)
		HAIER_LOGV(TAG, "Received data: %s", outBuf.str().c_str());
#endif
	return bytes_read;
}

void TransportLevelHandler::processData()
{
	if (mCurrentFrame.getStatus() > FrameStatus::fsEmpty)
	{
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - mFrameStart) > FRAME_TIMEOUT)
		{
			// Timeout
			HAIER_LOGW(TAG, "Frame timeout!");
			dropBytes(mPos);
			mPos = 0;
			mSepCount = 0;
			mFrameStartFound = false;
			mCurrentFrame.reset();
		}
	}
	if (!mBuffer.empty())
	{
		size_t bufSize = mBuffer.getAvailable();
		int bytesToDrop = 0;
		while (mPos < bufSize)
		{
			// Searching for the begging of packet
			if (!mFrameStartFound)
			{ 
				if (mBuffer[mPos] == SEPARATOR_BYTE)
				{
					if (mPos + 1 - bytesToDrop > FRAME_SEPARATORS_COUNT)
						bytesToDrop = mPos +1 - FRAME_SEPARATORS_COUNT;
				}
				else
				{
					if (mPos - bytesToDrop == FRAME_SEPARATORS_COUNT)
					{
						mFrameStartFound = true;
						if (bytesToDrop > 0)
						{
							// Dropping garbage
							dropBytes(bytesToDrop);
							bufSize -= bytesToDrop;
							mPos -= bytesToDrop;
							bytesToDrop = 0;
						}
					}
					else
						bytesToDrop = mPos + 1;
				}
			}
			else
			{
				if (mBuffer[mPos] == SEPARATOR_BYTE)
					++mSepCount;
				switch (mCurrentFrame.getStatus())
				{
					case FrameStatus::fsEmpty:
					{
						if (mPos + 1 - mSepCount == FRAME_HEADER_SIZE)
						{
							std::unique_ptr<uint8_t[]> headerBuffer(new uint8_t[FRAME_HEADER_SIZE]);
							size_t hPos = 0;
							size_t bPos = 0;
							for (; bPos < FRAME_SEPARATORS_COUNT; ++bPos)
								headerBuffer.get()[hPos++] = mBuffer[bPos];
							headerBuffer.get()[hPos++] = mBuffer[bPos++];
							while (bPos <= mPos)
							{
								if (mBuffer[bPos - 1] != SEPARATOR_BYTE)
									headerBuffer.get()[hPos++] = mBuffer[bPos];
								++bPos;
							}
							mBuffer.drop(bPos);
							bufSize -= mPos + 1;
							mPos = 0;
							mSepCount = 0;
							FrameError err;
							mCurrentFrame.parseBuffer(headerBuffer.get(), hPos, err);
							if (err == FrameError::feHeaderOnly)
								HAIER_LOGV(TAG, "Found frame header: type %02X", mCurrentFrame.getFrameType());
							else
							{
								HAIER_LOGW(TAG, "Frame parsing error: %d", err);
								mCurrentFrame.reset();
								mFrameStartFound = false;
							}
							continue;
						}
					}
					break;
				case FrameStatus::fsHeaderOnly:
					if (mPos + 1 - mSepCount == mCurrentFrame.getDataSize() + (mCurrentFrame.getUseCrc() ? 3 : 1))
					{
						std::unique_ptr<uint8_t[]> tmpBuf(new uint8_t[mPos + 1 - mSepCount]);
						size_t hPos = 0;
						size_t bPos = 0;
						while (bPos <= mPos)
						{
							if ((bPos == 0) || (mBuffer[bPos - 1] != SEPARATOR_BYTE))
								tmpBuf.get()[hPos++] = mBuffer[bPos];
							++bPos;
						}
						mBuffer.drop(bPos);
						FrameError err;
						mCurrentFrame.parseBuffer(tmpBuf.get(), hPos, err);
						if (err == FrameError::feCompleteFrame)
						{
							HAIER_LOGD(TAG, "Frame found: type %02X, data: %s", mCurrentFrame.getFrameType(), buf2hex(tmpBuf.get(), mCurrentFrame.getDataSize()).c_str());
							mIncommingQueue.push(TimestampedFrame{ std::move(mCurrentFrame), mFrameStart });
						}
						else
							HAIER_LOGW(TAG, "Frame parsing error: %d", err);
						mCurrentFrame.reset();
						mPos = 0;
						mSepCount = 0;
						bufSize = mBuffer.getAvailable();
						mFrameStartFound = false;
						continue;
					}
					break;
					// No need for separate FrameStatus::fsComplete
				default:
					// Shouldn't get here!
					mCurrentFrame.reset();
					mFrameStartFound = false;
					continue;
				}
			}
			mPos++;
		}
		if (bytesToDrop > 0)
		{
			dropBytes(bytesToDrop);
			mPos -= bytesToDrop;
			bytesToDrop = 0;
		}
	}
}

void TransportLevelHandler::resetProtocol() noexcept
{
	mCurrentFrame.reset();
	size_t bytesToDrop = mBuffer.getAvailable();
	if (mPos < bytesToDrop)
		bytesToDrop = mPos;
	dropBytes(bytesToDrop);
	mPos = 0;
	mSepCount = 0;
	mFrameStartFound = false;
}

 bool TransportLevelHandler::pop(TimestampedFrame& tframe)
{
	if (mIncommingQueue.empty())
		return false;
	tframe = std::move(mIncommingQueue.front());
	mIncommingQueue.pop();
	return true;
}

 void TransportLevelHandler::drop(size_t framesCount)
 {
	 size_t sz = mIncommingQueue.size();
	 if (framesCount < sz)
		 sz = framesCount;
	 while (sz-- > 0)
		 mIncommingQueue.pop();
 }

TransportLevelHandler::~TransportLevelHandler()
{
}

void TransportLevelHandler::clear()
{
	HAIER_LOGV(TAG, "Clearing buffer, data size: %d", mBuffer.getAvailable());
	mBuffer.clear();
	mPos = 0;
	mSepCount = 0;
	mFrameStartFound = false;
	mCurrentFrame.reset();
}

void TransportLevelHandler::dropBytes(size_t size)
{
	mBuffer.drop(size);
	HAIER_LOGV(TAG, "Dropping %d bytes", size);
}

} // HaierProtocol