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
	mCurrentFrame(std::move(source.mCurrentFrame))
{
}

TransportLevelHandler::TransportLevelHandler(ProtocolStream& stream, size_t bufferSize) noexcept :
	mStream(stream),
	mBuffer(bufferSize),
	mPos(0),
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
	std::stringstream outBuf;
	while ((bytes_read < count) && (mStream.read_array(&val, 1) > 0))
	{
		outBuf << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int)val << ' ';
		if (mBuffer.push(val) == 0)
			break;
		bytes_read++;
	}
	if (bytes_read > 0)
		HAIER_LOGV(TAG, "Received data: %s", outBuf.str().c_str());
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
			mCurrentFrame.reset();
		}
	}
	if (!mBuffer.empty())
	{
		size_t bufSize = mBuffer.getAvailable();
		int bytesToDrop = 0;
		while (mPos < bufSize)
		{
			switch (mCurrentFrame.getStatus())
			{
			case FrameStatus::fsEmpty:
			{
				if (mPos - bytesToDrop + 1 == FRAME_HEADER_SIZE)
				{
					size_t sepCounter = 0;
					for (; sepCounter < FRAME_SEPARATORS_COUNT; ++sepCounter)
						if (mBuffer[bytesToDrop + sepCounter] != SEPARATOR_BYTE)
							break;
					if (sepCounter < FRAME_SEPARATORS_COUNT)
						bytesToDrop += (int)(sepCounter + 1);
					else if (mBuffer[bytesToDrop + FRAME_SEPARATORS_COUNT] == SEPARATOR_BYTE)
						++bytesToDrop;
					else
					{
						// Found packet start
						if (bytesToDrop > 0)
						{
							// Dropping garbage
							dropBytes(bytesToDrop);
							bufSize -= bytesToDrop;
							mPos -= bytesToDrop;
							bytesToDrop = 0;
						}
						std::unique_ptr<uint8_t[]> headerBuffer(new uint8_t[mPos + 1]);
						mBuffer.pop(headerBuffer.get(), mPos + 1);
						FrameError err;
						mCurrentFrame.parseBuffer(headerBuffer.get(), mPos + 1, err);
						if (err == FrameError::feHeaderOnly)
							HAIER_LOGV(TAG, "Found frame header: type %d", mCurrentFrame.getFrameType());
						else
						{
							HAIER_LOGW(TAG, "Frame parsing error: %d", err);
							mCurrentFrame.reset();
						}
						mPos = 0;
						bufSize = mBuffer.getAvailable();
						continue;
					}
				}
			}
			break;
			case FrameStatus::fsHeaderOnly:
				if (mPos + 1 == mCurrentFrame.getDataSize() + (mCurrentFrame.getUseCrc() ? 3 : 1))
				{
					std::unique_ptr<uint8_t[]> tmpBuf(new uint8_t[mPos + 1]);
					mBuffer.pop(tmpBuf.get(), mPos + 1);
					FrameError err;
					mCurrentFrame.parseBuffer(tmpBuf.get(), mPos + 1, err);
					if (err == FrameError::feCompleteFrame)
					{
						HAIER_LOGD(TAG, "Frame found: type %d, data: %s", mCurrentFrame.getFrameType(), buf2hex(tmpBuf.get(), mCurrentFrame.getDataSize()).c_str());
						mIncommingQueue.push(TimestampedFrame{ std::move(mCurrentFrame), mFrameStart });
					}
					else
						HAIER_LOGW(TAG, "Frame parsing error: %d", err);
					mCurrentFrame.reset();
					mPos = 0;
					bufSize = mBuffer.getAvailable();
					continue;
				}
				break;
			case FrameStatus::fsComplete:
				// Shouldn't get here!
				mCurrentFrame.reset();
				continue;
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
	mCurrentFrame.reset();
}

void TransportLevelHandler::dropBytes(size_t size)
{
	mBuffer.drop(size);
	HAIER_LOGV(TAG, "Dropping %d bytes", size);
}

} // HaierProtocol