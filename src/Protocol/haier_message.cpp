#include "haier_protocol.h"
#include <memory>

#define TAG "haier.protocol"

namespace HaierProtocol
{

HaierMessage::HaierMessage() noexcept :
	HaierMessage(UNKNOWN_MESSAGE_TYPE, NO_SUBCOMMAND, nullptr, 0)
{
}

HaierMessage::HaierMessage(uint8_t frameType) noexcept :
	HaierMessage(frameType, NO_SUBCOMMAND, nullptr, 0)
{
}

HaierMessage::HaierMessage(uint8_t frameType, uint16_t subcommand) noexcept :
	HaierMessage(frameType, subcommand, nullptr, 0)
{
}

HaierMessage::HaierMessage(uint8_t frameType, const uint8_t* data, size_t dataSize) noexcept :
	HaierMessage(frameType, NO_SUBCOMMAND, data, dataSize)
{
}

HaierMessage::HaierMessage(uint8_t frameType, uint16_t subcommand, const uint8_t* data, size_t dataSize) noexcept :
	mFrameType(frameType),
	mSubcommand(subcommand),
	mDataSize(dataSize),
	mData(nullptr)
{
	if ((data != nullptr) && (dataSize != 0))
	{
		mData = new uint8_t[mDataSize];
		memcpy(mData, data, mDataSize);
	}
	else
		mDataSize = 0;
}

HaierMessage::HaierMessage(const HaierMessage& source) noexcept :
	mFrameType(source.mFrameType),
	mSubcommand(source.mSubcommand),
	mDataSize(source.mDataSize)
{
	mData = new uint8_t[mDataSize];
	memcpy(mData, source.mData, mDataSize);
}

HaierMessage::HaierMessage(HaierMessage&& source) noexcept :
	mFrameType(source.mFrameType),
	mSubcommand(source.mSubcommand),
	mDataSize(source.mDataSize),
	mData(source.mData)
{
	source.mData = nullptr;
}

HaierMessage::~HaierMessage() noexcept
{
	if (mData != nullptr)
		delete[] mData;
}

HaierMessage& HaierMessage::operator=(const HaierMessage& source) noexcept
{
	if (this != &source)
	{
		mFrameType = source.mFrameType;
		mSubcommand = source.mSubcommand;
		mDataSize = source.mDataSize;
		mData = new uint8_t[mDataSize];
		memcpy(mData, source.mData, mDataSize);
	}
	return *this;
}

HaierMessage& HaierMessage::operator=(HaierMessage&& source) noexcept
{
	if (this != &source)
	{
		mFrameType = source.mFrameType;
		mSubcommand = source.mSubcommand;
		mDataSize = source.mDataSize;
		mData = source.mData;
		source.mData = nullptr;
	}
	return *this;

}

size_t HaierMessage::fillBuffer(uint8_t* const buffer, size_t limit) const
{
	if (getBuferSize() > limit)
		return 0;
	size_t pos = 0;
	if (mSubcommand != NO_SUBCOMMAND)
	{
		buffer[pos++] = mSubcommand >> 8;
		buffer[pos++] = mSubcommand & 0xFF;
	}
	if (mDataSize > 0)
		memcpy(buffer + pos, mData, mDataSize);
	return pos + mDataSize;
}

} // HaierProtocol