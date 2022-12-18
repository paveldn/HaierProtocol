#ifndef HAIRER_MESSAGE_H
#define HAIRER_MESSAGE_H

#include <cstdint>

namespace HaierProtocol
{

constexpr auto UNKNOWN_MESSAGE_TYPE     = 0x00;
constexpr auto NO_SUBCOMMAND            = 0x0000;

class HaierMessage
{
public:
    HaierMessage() noexcept;
    explicit HaierMessage(uint8_t frameType) noexcept;
    HaierMessage(uint8_t frameType, uint16_t subcommand) noexcept;
    HaierMessage(uint8_t frameType, const uint8_t* data, size_t dataSize) noexcept;
    HaierMessage(uint8_t frameType, uint16_t subcommand, const uint8_t* data, size_t dataSize) noexcept;
    HaierMessage(const HaierMessage&) noexcept;
    HaierMessage(HaierMessage&&) noexcept;
    virtual ~HaierMessage() noexcept;
    HaierMessage& operator=(const HaierMessage&) noexcept;
    HaierMessage& operator=(HaierMessage&&) noexcept;
    uint8_t getFrameType() const { return mFrameType; };
    uint16_t getSubcommand() const { return mSubcommand; };
    size_t getDataSize() const { return mDataSize; };
    const uint8_t* getData() const { return mData; };
    size_t fillBuffer(uint8_t* const buffer, size_t limit) const;
    size_t getBuferSize() const { return mDataSize + ((mSubcommand == NO_SUBCOMMAND) ? 0 : 2); }
private:
    uint8_t     mFrameType;
    uint16_t    mSubcommand;
    size_t      mDataSize;
    uint8_t*    mData;
};

} // HaierProtocol
#endif // HAIRER_MESSAGE_H