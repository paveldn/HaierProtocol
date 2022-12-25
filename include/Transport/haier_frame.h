#ifndef HAIER_FRAME_H
#define HAIER_FRAME_H

#include <stdint.h>

namespace haier_protocol
{

constexpr uint8_t MAX_FRAME_SIZE          = 0xF1;
constexpr uint8_t FRAME_HEADER_SIZE       = 0x0A;
constexpr uint8_t SEPARATOR_BYTE          = 0xFF;
constexpr uint8_t SEPARATOR_POST_BYTE     = 0x55;
constexpr uint8_t FRAME_SEPARATORS_COUNT  = 0x02;
constexpr uint8_t PURE_HEADER_SIZE        = FRAME_HEADER_SIZE - FRAME_SEPARATORS_COUNT;

enum class FrameStatus
{
    FRAME_COMPLETE,
    FRAME_HEADER_ONLY,
    FRAME_EMPTY
};

enum class FrameError
{
    COMPLETE_FRAME,
    HEADER_ONLY,
    FRAME_SEPARATOR_WRONG,
    HEADER_TOO_SMALL,
    FRAME_TOO_BIG,
    FRAME_TOO_SMALL,
    DATA_SIZE_WRONG,
    CHECKSUM_WRONG,
    CRC_WRONG,
    WRONG_POST_SEPARATOR_BYTE,
    UNKNOWN_DATA,
};

class HaierFrame
{
public:
    HaierFrame() noexcept;
    HaierFrame(const HaierFrame&) = delete;
    HaierFrame& operator=(const HaierFrame&) = delete;
    HaierFrame& operator=(HaierFrame&&) noexcept;
    HaierFrame(uint8_t frame_type, const uint8_t* const data, uint8_t data_size, bool use_crc = true);
    HaierFrame(HaierFrame&&) noexcept;
    virtual ~HaierFrame() noexcept;
    FrameStatus         get_status() const { return this->status_; };
    uint8_t             get_frame_type() const { return this->frame_type_; };
    bool                get_use_crc() const { return this->use_crc_; };
    uint8_t             get_data_size() const { return this->data_size_; };
    uint8_t             get_checksum() const { return this->checksum_; };
    uint16_t            get_crc() const { return this->crc_; };
    size_t              get_buffer_size() const;
    const uint8_t*      get_data() const { return this->data_; };
    size_t              fill_buffer(uint8_t* const buffer, size_t limit) const;
    size_t              parse_buffer(const uint8_t* const buffer, size_t size, FrameError& err);
    void                reset();
protected:
    uint8_t             frame_type_;
    bool                use_crc_;
    uint8_t             data_size_;
    uint8_t             checksum_;
    uint16_t            crc_;
    uint8_t*            data_;
    FrameStatus         status_;
    uint8_t             additional_bytes_;
    uint8_t             get_header_byte_(uint8_t pos) const;
};

} // HaierProtocol
#endif // HAIER_FRAME_H
