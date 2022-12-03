#include <cstring>
#include "haier_frame.h"

#define USE_CRC_MASK			0x40
#define HEADER_SIZE_POS			0x02
#define HEADER_CRC_FLAG_POS		0x03
#define HEADER_FRAME_TYPE_POS	0x09
#define INITIAL_CRC				0x00

uint16_t crc16(const uint8_t* const data, size_t size, uint16_t initial_val = 0)
{
    constexpr uint16_t poly = 0xA001;
    uint16_t crc = initial_val;
    for (size_t i = 0; i < size; ++i)
    {
        crc ^= (unsigned short)data[i];
        for (int b = 0; b < 8; ++b)
        {
            if ((crc & 1) != 0)
            {
                crc >>= 1;
                crc ^= poly;
            }
            else
                crc >>= 1;
        }
    }
    return crc;
}

uint8_t checksum(const uint8_t* const data, size_t size, uint8_t initial_val = 0)
{
    uint8_t result = initial_val;
    for (int i = 0; i < size; i++) {
        result += data[i];
    }
    return result;
}

namespace HaierProtocol
{

uint8_t	HaierFrame::getHeaderByte(uint8_t pos) const
{
	if (pos < HEADER_SIZE_POS)
		return SEPARATOR_BYTE;
	switch (pos)
	{
		case HEADER_SIZE_POS:
			return PURE_HEADER_SIZE + mDataSize;
		case HEADER_CRC_FLAG_POS:
			return mUseCrc ? USE_CRC_MASK : 0;
		case HEADER_FRAME_TYPE_POS:
			return mFrameType;
		default:
			return 0;
	}
}

HaierProtocol::HaierFrame::HaierFrame() noexcept :
	mFrameType(0),
	mUseCrc(true),
	mDataSize(0),
	mChecksum(0),
	mCrc(INITIAL_CRC),
	mData(nullptr),
	mStatus(FrameStatus::fsEmpty)
{
}

HaierFrame::HaierFrame(uint8_t frameType, const uint8_t* const data, uint8_t dataSize, bool useCrc) noexcept :
	mFrameType(frameType),
	mUseCrc(useCrc),
	mDataSize(dataSize),
	mChecksum(0),
	mCrc(INITIAL_CRC),
	mData(nullptr),
	mStatus(FrameStatus::fsComplete)
{
	for (uint8_t pos = HEADER_SIZE_POS; pos < FRAME_HEADER_SIZE; pos++)
	{
		uint8_t hByte = getHeaderByte(pos);
		if (hByte > 0)
			mChecksum = checksum(&hByte, 1, mChecksum);
		if (mUseCrc)
			mCrc = crc16(&hByte, 1, mCrc);
	}
	if (mDataSize > 0)
	{
		mData = new uint8_t[mDataSize];
		memcpy(mData, data, mDataSize);
		mChecksum = checksum(mData, mDataSize, mChecksum);
		if (mUseCrc)
			mCrc = crc16(mData, mDataSize, mCrc);
	}
}

HaierFrame::HaierFrame(HaierFrame&& source) noexcept :
	mFrameType(source.mFrameType),
	mUseCrc(source.mUseCrc),
	mDataSize(source.mDataSize),
	mChecksum(source.mChecksum),
	mCrc(source.mCrc),
	mData(source.mData),
	mStatus(source.mStatus)
{
	source.mData = nullptr;
}

HaierFrame::~HaierFrame() noexcept
{
	if (mData != nullptr)
	{
		delete[] mData;
		mData = nullptr;
	}
}

HaierFrame& HaierFrame::operator=(HaierFrame&& source) noexcept
{
	if (this != &source)
	{
		mFrameType = source.mFrameType;
		mUseCrc = source.mUseCrc;
		mDataSize = source.mDataSize;
		mChecksum = source.mChecksum;
		mCrc = source.mCrc;
		mStatus = source.mStatus;
		if (mData != nullptr)
			delete[] mData;
		mData = source.mData;
		source.mData = nullptr;
	}
	return *this;
}

size_t HaierFrame::parseBuffer(const uint8_t* const buffer, size_t size, FrameError& err)
{
	size_t lPos = 0;
	if (mStatus == FrameStatus::fsComplete)
	{
		err = FrameError::feUnknownData;
		return lPos;
	}
	if (mStatus == FrameStatus::fsEmpty)
	{
		// Parsing header
		if (size < FRAME_HEADER_SIZE)
		{
			err = FrameError::feHeaderTooSmall;
			return lPos;
		}
		for (; lPos < FRAME_SEPARATORS_COUNT; lPos++)
			if (buffer[lPos] != SEPARATOR_BYTE)
			{
				err = FrameError::feFrameSeparatorWrong;
				return 0;
			}
		uint8_t fsize = 0;
		bool useCrc = true;
		uint8_t frameType = 0;
		uint8_t chk = 0;
		uint16_t crc = INITIAL_CRC;
		while (lPos < FRAME_HEADER_SIZE)
		{
			switch (lPos)
			{
			case HEADER_SIZE_POS:
				fsize = buffer[lPos];
				break;
			case HEADER_CRC_FLAG_POS:
				useCrc = (buffer[lPos] & USE_CRC_MASK) != 0;
				if (!useCrc)
					crc = INITIAL_CRC;
				break;
			case HEADER_FRAME_TYPE_POS:
				frameType = buffer[lPos];
				break;
			}
			chk = checksum(buffer + lPos, 1, chk);
			if (useCrc)
				crc = crc16(buffer + lPos, 1, crc);
			lPos++;
		}
		if (fsize < PURE_HEADER_SIZE)
		{
			err = FrameError::feFrameTooSmall;
			return lPos;
		}
			
		if (fsize > MAX_FRAME_SIZE)
		{
			err = FrameError::feFrameTooBig;
			return lPos;
		}
		mFrameType = frameType;
		mUseCrc = useCrc;
		mDataSize = fsize - PURE_HEADER_SIZE;
		mChecksum = chk;
		mCrc = crc;
		mStatus = FrameStatus::fsHeaderOnly;
		if (lPos == size)
		{
			err = FrameError::feHeaderOnly;
			return lPos;
		}
	}
	if (mStatus == FrameStatus::fsHeaderOnly)
	{
		uint8_t minSize = mDataSize + (mUseCrc ? 3 : 1);
		if (size - lPos < minSize)
		{
			err = FrameError::feDataSizeWrong;
			return lPos;
		}
		size_t rPos = minSize - 1;
		uint16_t crc = INITIAL_CRC;
		if (mUseCrc)
		{
			uint16_t frameCrc = buffer[rPos] + (buffer[rPos - 1] << 8);
			crc = crc16(buffer, mDataSize, mCrc);
			if (crc != frameCrc)
			{
				err = FrameError::feCrcWrong;
				return lPos;
			}
			rPos -= 2;
		}
		uint8_t chk = checksum(buffer + lPos, mDataSize, mChecksum);
		if (chk != buffer[rPos])
		{
			err = FrameError::feChecksumWrong;
			return lPos;
		}
		mCrc = crc;
		mChecksum = chk;
		mStatus = FrameStatus::fsComplete;
		if (mDataSize != 0)
		{
			mData = new uint8_t[mDataSize];
			memcpy(mData, buffer + lPos, mDataSize);
		}
		else
			mData = nullptr;
		err = FrameError::feCompleteFrame;
		return lPos;
	}
	err = FrameError::feUnknownData;
	return lPos;
}

void HaierFrame::reset()
{
	if (mData != nullptr)
		delete[] mData;
	mFrameType = 0;
	mUseCrc = true;
	mDataSize = 0;
	mChecksum = 0;
	mCrc = INITIAL_CRC;
	mData = nullptr;
	mStatus = FrameStatus::fsEmpty;
}

size_t HaierFrame::getBufferSize() const
{
	// FF FF <header> <data> <checksum> [CRC]
	return FRAME_HEADER_SIZE + mDataSize + (mUseCrc ? 3 : 1);
}

size_t HaierFrame::fillBuffer(uint8_t* const buffer, size_t limit) const
{
	size_t sz = getBufferSize();
	if (sz > limit)
		return 0;
	uint8_t pos = 0;
	for (; pos < FRAME_HEADER_SIZE; pos++)
		buffer[pos] = getHeaderByte(pos);
	memcpy(buffer + pos, mData, mDataSize);
	pos = sz - 1;
	if (mUseCrc)
	{
		buffer[pos--] = mCrc & 0xFF;
		buffer[pos--] = mCrc >> 8;
	}
	buffer[pos] = mChecksum;
	return sz;
}

}