#ifndef HAIER_FRAME_H
#define HAIER_FRAME_H

#include <stdint.h>

#define MAX_FRAME_SIZE			0xF1
#define FRAME_HEADER_SIZE		0x0A
#define SEPARATOR_BYTE			0xFF
#define SEPARATOR_POST_BYTE		0x55
#define FRAME_SEPARATORS_COUNT	0x02
#define PURE_HEADER_SIZE		((FRAME_HEADER_SIZE) - (FRAME_SEPARATORS_COUNT))

namespace HaierProtocol
{

enum class FrameStatus
{
	fsComplete,
	fsHeaderOnly,
	fsEmpty
};

enum class FrameError
{
	feCompleteFrame,
	feHeaderOnly,
	feFrameSeparatorWrong,
	feHeaderTooSmall,
	feFrameTooBig,
	feFrameTooSmall,
	feDataSizeWrong,
	feChecksumWrong,
	feCrcWrong,
	feWrongPostSeparatorByte,
	feUnknownData,
};

class HaierFrame
{
public:
	HaierFrame() noexcept;
	HaierFrame(const HaierFrame&) = delete;
	HaierFrame& operator=(const HaierFrame&) = delete;
	HaierFrame& operator=(HaierFrame&&) noexcept;
	HaierFrame(uint8_t frameType, const uint8_t* const data, uint8_t dataSize, bool useCrc = true) noexcept;
	HaierFrame(HaierFrame&&) noexcept;
	virtual ~HaierFrame() noexcept;
	FrameStatus			getStatus() const { return mStatus; };
	uint8_t				getFrameType() const { return mFrameType; };
	bool				getUseCrc() const { return mUseCrc; };
	uint8_t				getDataSize() const { return mDataSize; };
	uint8_t				getChecksum() const { return mChecksum; };
	uint16_t			getCrc() const { return mCrc; };
	size_t				getBufferSize() const;
	const uint8_t*		getData() const { return mData; };
	size_t 				fillBuffer(uint8_t* const buffer, size_t limit) const;
	size_t				parseBuffer(const uint8_t* const buffer, size_t size, FrameError& err);
	void				reset();
private:
	uint8_t				mFrameType;
	bool				mUseCrc;
	uint8_t				mDataSize;
	uint8_t				mChecksum;
	uint16_t			mCrc;
	uint8_t*			mData;
	FrameStatus			mStatus;
	uint8_t				mAdditionalBytes;
	uint8_t				getHeaderByte(uint8_t pos) const;
};

} // HaierProtocol
#endif // HAIER_FRAME_H
