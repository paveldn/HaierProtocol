#include <cstring>
#include <cstring>
#include "Transport/haier_frame.h"

#define USE_CRC_MASK            0x40
#define HEADER_SIZE_POS         0x02
#define HEADER_CRC_FLAG_POS     0x03
#define HEADER_FRAME_TYPE_POS   0x09
#define INITIAL_CRC             0x00

// CRC-16/ARC lookup table
static const uint16_t crc_table[] = {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040,
    };


inline uint16_t crc16(const uint8_t data, uint16_t crc = 0)
{
    return (crc >> 8) ^ crc_table[(crc ^ data) & 0xFF];
}

uint16_t crc16(const uint8_t* const data, size_t size, uint16_t initial_val = 0)
{
    const uint8_t* val = data;
    uint16_t crc = initial_val;
    while (size--)
    {
        crc = crc16(*val, crc);
        ++val;
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

uint8_t HaierFrame::getHeaderByte(uint8_t pos) const
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
    mStatus(FrameStatus::fsEmpty),
    mAdditionalBytes(0)
{
}

HaierFrame::HaierFrame(uint8_t frameType, const uint8_t* const data, uint8_t dataSize, bool useCrc) noexcept :
    mFrameType(frameType),
    mUseCrc(useCrc),
    mDataSize(dataSize),
    mChecksum(0),
    mCrc(INITIAL_CRC),
    mData(nullptr),
    mStatus(FrameStatus::fsComplete),
    mAdditionalBytes(0)
{
    for (uint8_t pos = HEADER_SIZE_POS; pos < FRAME_HEADER_SIZE; pos++)
    {
        uint8_t hByte = getHeaderByte(pos);
        if (hByte == SEPARATOR_BYTE)
            ++mAdditionalBytes;
        if (hByte > 0)
            mChecksum += hByte;
        if (mUseCrc)
            mCrc = crc16(hByte, mCrc);
    }
    if (mDataSize > 0)
    {
        mData = new uint8_t[mDataSize];
        memcpy(mData, data, mDataSize);
        for (size_t i = 0; i < mDataSize; i++)
        {
            if (mData[i] > 0)
                mChecksum += mData[i];
            if (mData[i] == SEPARATOR_BYTE)
                ++mAdditionalBytes;
            if (mUseCrc)
                mCrc = crc16(mData[i], mCrc);
        }
    }
    mChecksum += mAdditionalBytes * SEPARATOR_POST_BYTE;
}

HaierFrame::HaierFrame(HaierFrame&& source) noexcept :
    mFrameType(source.mFrameType),
    mUseCrc(source.mUseCrc),
    mDataSize(source.mDataSize),
    mChecksum(source.mChecksum),
    mCrc(source.mCrc),
    mData(source.mData),
    mStatus(source.mStatus),
    mAdditionalBytes(source.mAdditionalBytes)
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
        mAdditionalBytes = source.mAdditionalBytes;
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
                mAdditionalBytes = (frameType == SEPARATOR_BYTE) ? 1 : 0;
                break;
            }
            chk = checksum(buffer + lPos, 1, chk);
            if (useCrc)
                crc = crc16(buffer[lPos], crc);
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
        for (size_t i = 0; i < mDataSize; i++)
            if (buffer[i] == SEPARATOR_BYTE)
                ++mAdditionalBytes;
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
        uint8_t chk = checksum(buffer + lPos, mDataSize, mChecksum) + ((mAdditionalBytes * SEPARATOR_POST_BYTE) & 0xFF);
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
    mAdditionalBytes = 0;
}

size_t HaierFrame::getBufferSize() const
{
    // FF FF <header> <data> <checksum> [CRC]
    size_t result = FRAME_HEADER_SIZE + mDataSize + mAdditionalBytes;
    if (mChecksum == SEPARATOR_BYTE)
        result += 2;
    else
        result += 1;
    if (mUseCrc)
    {
        result += 2;
        if ((mCrc & 0xFF) == SEPARATOR_BYTE)
            ++result;
        if (((mCrc >> 8) & 0xFF) == SEPARATOR_BYTE)
            ++result;
    }
    return  result;
}

size_t HaierFrame::fillBuffer(uint8_t* const buffer, size_t limit) const
{
#define SET_WITH_POST_BYTE(value, dst, position)    do {\
                (dst)[(position)++] = (value); \
                if ((value) == SEPARATOR_BYTE) \
                { \
                    (dst)[(position)++] = SEPARATOR_POST_BYTE; \
                } \
            } while (0)
    size_t sz = getBufferSize();
    if (sz > limit)
        return 0;
    uint8_t pos = 0;
    for (size_t i = 0; i < FRAME_SEPARATORS_COUNT; i++)
        buffer[pos++] = getHeaderByte(i);
    for (size_t i = FRAME_SEPARATORS_COUNT; i < FRAME_HEADER_SIZE; i++)
    {

        uint8_t val = getHeaderByte(i);
        SET_WITH_POST_BYTE(val, buffer, pos);
    }
    for (size_t i = 0; i < mDataSize; i++)
    {
        uint8_t val = mData[i];
        SET_WITH_POST_BYTE(val, buffer, pos);
    }
    SET_WITH_POST_BYTE(mChecksum, buffer, pos);
    if (mUseCrc)
    {
        uint8_t val = (mCrc >> 8) & 0xFF;
        SET_WITH_POST_BYTE(val, buffer, pos);
        val = mCrc & 0xFF;
        SET_WITH_POST_BYTE(val, buffer, pos);
    }
    return sz;
#undef SET_WITH_POST_BYTE
}

}