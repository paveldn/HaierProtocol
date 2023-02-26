#ifndef HAIRER_MESSAGE_H
#define HAIRER_MESSAGE_H

#include <cstdint>

namespace haier_protocol
{

constexpr uint8_t UNKNOWN_MESSAGE_TYPE     = 0x00;
constexpr uint16_t NO_SUBCOMMAND           = 0x0000;

class HaierMessage
{
public:
    HaierMessage() noexcept;
    explicit HaierMessage(uint8_t frame_type) noexcept;
    HaierMessage(uint8_t frame_type, uint16_t subcommand) noexcept;
    HaierMessage(uint8_t frame_type, const uint8_t* data, size_t data_size) noexcept;
    HaierMessage(uint8_t frame_type, uint16_t subcommand, const uint8_t* data, size_t data_size);
    HaierMessage(const HaierMessage&);
    HaierMessage(HaierMessage&&) noexcept;
    virtual ~HaierMessage() noexcept;
    HaierMessage& operator=(const HaierMessage&);
    HaierMessage& operator=(HaierMessage&&) noexcept;
    uint8_t get_frame_type() const { return this->frame_type_; };
    uint16_t get_sub_command() const { return this->subcommand_; };
    size_t get_data_size() const { return this->data_size_; };
    const uint8_t* get_data() const { return this->data_; };
    size_t fill_buffer(uint8_t* const buffer, size_t limit) const;
    size_t get_bufer_size() const { return this->data_size_ + ((this->subcommand_ == NO_SUBCOMMAND) ? 0 : 2); }
protected:
    uint8_t     frame_type_;
    uint16_t    subcommand_;
    size_t      data_size_;
    uint8_t*    data_;
};

} // HaierProtocol
#endif // HAIRER_MESSAGE_H