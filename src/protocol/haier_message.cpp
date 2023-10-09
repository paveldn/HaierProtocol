#include <cstring>
#include "protocol/haier_message.h"

namespace haier_protocol
{

HaierMessage::HaierMessage() noexcept : HaierMessage(FrameType::UNKNOWN_FRAME_TYPE, NO_SUBCOMMAND, nullptr, 0)
{
}

HaierMessage::HaierMessage(FrameType frame_type) noexcept : HaierMessage(frame_type, NO_SUBCOMMAND, nullptr, 0)
{
}

HaierMessage::HaierMessage(FrameType frame_type, uint16_t subcommand) noexcept : HaierMessage(frame_type, subcommand, nullptr, 0)
{
}

HaierMessage::HaierMessage(FrameType frame_type, const uint8_t *data, size_t data_size) noexcept : HaierMessage(frame_type, NO_SUBCOMMAND, data, data_size)
{
}

HaierMessage::HaierMessage(FrameType frame_type, uint16_t subcommand, const uint8_t *data, size_t data_size) : frame_type_(frame_type),
  subcommand_(subcommand),
  data_size_(data_size),
  data_(nullptr)
{
  if ((data != nullptr) && (data_size != 0))
  {
    this->data_ = new uint8_t[data_size_];
    memcpy(this->data_, data, this->data_size_);
  }
  else
    this->data_size_ = 0;
}

HaierMessage::HaierMessage(const HaierMessage &source) : frame_type_(source.frame_type_),
  subcommand_(source.subcommand_),
  data_size_(source.data_size_)
{
  this->data_ = new uint8_t[this->data_size_];
  memcpy(this->data_, source.data_, this->data_size_);
}

HaierMessage::HaierMessage(HaierMessage &&source) noexcept : frame_type_(source.frame_type_),
  subcommand_(source.subcommand_),
  data_size_(source.data_size_),
  data_(source.data_)
{
  source.data_ = nullptr;
}

HaierMessage::~HaierMessage() noexcept
{
  delete[] this->data_;
}

HaierMessage &HaierMessage::operator=(const HaierMessage &source)
{
  if (this != &source)
  {
    this->frame_type_ = source.frame_type_;
    this->subcommand_ = source.subcommand_;
    this->data_size_ = source.data_size_;
    this->data_ = new uint8_t[data_size_];
    memcpy(this->data_, source.data_, this->data_size_);
  }
  return *this;
}

HaierMessage &HaierMessage::operator=(HaierMessage &&source) noexcept
{
  if (this != &source)
  {
    this->frame_type_ = source.frame_type_;
    this->subcommand_ = source.subcommand_;
    this->data_size_ = source.data_size_;
    this->data_ = source.data_;
    source.data_ = nullptr;
  }
  return *this;
}

size_t HaierMessage::fill_buffer(uint8_t *const buffer, size_t limit) const
{
  if (this->get_buffer_size() > limit)
    return 0;
  size_t pos = 0;
  if (this->subcommand_ != NO_SUBCOMMAND)
  {
    buffer[pos++] = this->subcommand_ >> 8;
    buffer[pos++] = this->subcommand_ & 0xFF;
  }
  if (this->data_size_ > 0)
    memcpy(buffer + pos, this->data_, this->data_size_);
  return pos + this->data_size_;
}

} // haier_protocol