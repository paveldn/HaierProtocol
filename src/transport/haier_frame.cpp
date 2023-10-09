#include <cstring>
#include <cstring>
#include "transport/haier_frame.h"

namespace haier_protocol
{

constexpr uint8_t USE_CRC_MASK            = 0x40;
constexpr uint8_t  HEADER_SIZE_POS        = 0x02;
constexpr uint8_t  HEADER_CRC_FLAG_POS    = 0x03;
constexpr uint8_t  HEADER_FRAME_TYPE_POS  = 0x09;
constexpr uint8_t  INITIAL_CRC            = 0x00;

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

uint8_t HaierFrame::get_header_byte_(uint8_t pos) const
{
  if (pos < HEADER_SIZE_POS)
    return SEPARATOR_BYTE;
  switch (pos)
  {
    case HEADER_SIZE_POS:
      return PURE_HEADER_SIZE + this->data_size_;
    case HEADER_CRC_FLAG_POS:
      return  this->use_crc_ ? USE_CRC_MASK : 0;
    case HEADER_FRAME_TYPE_POS:
      return (uint8_t) this->frame_type_;
    default:
      return 0;
    }
}

haier_protocol::HaierFrame::HaierFrame() noexcept :
  frame_type_(0),
  use_crc_(true),
  data_size_(0),
  checksum_(0),
  crc_(INITIAL_CRC),
  data_(nullptr),
  status_(FrameStatus::FRAME_EMPTY),
  additional_bytes_(0)
{
}

HaierFrame::HaierFrame(uint8_t frame_type, const uint8_t* const data, uint8_t data_size, bool use_crc) :
  frame_type_(frame_type),
  use_crc_(use_crc),
  data_size_(data_size),
  checksum_(0),
  crc_(INITIAL_CRC),
  data_(nullptr),
  status_(FrameStatus::FRAME_COMPLETE),
  additional_bytes_(0)
{
  for (uint8_t pos = HEADER_SIZE_POS; pos < FRAME_HEADER_SIZE; pos++)
  {
    uint8_t hbyte = this->get_header_byte_(pos);
    if (hbyte == SEPARATOR_BYTE)
      ++this->additional_bytes_;
    if (hbyte > 0)
      this->checksum_ += hbyte;
    if (this->use_crc_)
      this->crc_ = crc16(hbyte, crc_);
  }
  if (this->data_size_ > 0)
  {
    this->data_ = new uint8_t[this->data_size_];
    memcpy(this->data_, data, this->data_size_);
    for (size_t i = 0; i < this->data_size_; i++)
    {
      if (this->data_[i] > 0)
        this->checksum_ += this->data_[i];
      if (this->data_[i] == SEPARATOR_BYTE)
        ++this->additional_bytes_;
      if (this->use_crc_)
        this->crc_ = crc16(this->data_[i], this->crc_);
    }
  }
  this->checksum_ += this->additional_bytes_ * SEPARATOR_POST_BYTE;
}

HaierFrame::HaierFrame(HaierFrame&& source) noexcept :
  frame_type_(source.frame_type_),
  use_crc_(source.use_crc_),
  data_size_(source.data_size_),
  checksum_(source.checksum_),
  crc_(source.crc_),
  data_(source.data_),
  status_(source.status_),
  additional_bytes_(source.additional_bytes_)
{
  source.data_ = nullptr;
}

HaierFrame::~HaierFrame() noexcept
{
  if (this->data_ != nullptr)
  {
    delete[] this->data_;
    this->data_ = nullptr;
  }
}

HaierFrame& HaierFrame::operator=(HaierFrame&& source) noexcept
{
  if (this != &source)
  {
    this->frame_type_ = source.frame_type_;
    this->use_crc_ = source.use_crc_;
    this->data_size_ = source.data_size_;
    this->checksum_ = source.checksum_;
    this->crc_ = source.crc_;
    this->status_ = source.status_;
    delete[] this->data_;
    this->data_ = source.data_;
    this->additional_bytes_ = source.additional_bytes_;
    source.data_ = nullptr;
  }
  return *this;
}

size_t HaierFrame::parse_buffer(const uint8_t* const buffer, size_t size, FrameError& err)
{
  size_t lpos = 0;
  if (this->status_ == FrameStatus::FRAME_COMPLETE)
  {
    err = FrameError::UNKNOWN_DATA;
    return lpos;
  }
  if (this->status_ == FrameStatus::FRAME_EMPTY)
  {
    // Parsing header
    if (size < FRAME_HEADER_SIZE)
    {
      err = FrameError::HEADER_TOO_SMALL;
      return lpos;
    }
    for (; lpos < FRAME_SEPARATORS_COUNT; lpos++)
      if (buffer[lpos] != SEPARATOR_BYTE)
      {
        err = FrameError::FRAME_SEPARATOR_WRONG;
        return 0;
      }
    uint8_t fsize = 0;
    bool use_crc = true;
    uint8_t frame_type = 0;
    uint8_t chk = 0;
    uint16_t crc = INITIAL_CRC;
    while (lpos < FRAME_HEADER_SIZE)
    {
      switch (lpos)
      {
      case HEADER_SIZE_POS:
        fsize = buffer[lpos];
        break;
      case HEADER_CRC_FLAG_POS:
        use_crc = (buffer[lpos] & USE_CRC_MASK) != 0;
        if (!use_crc)
          crc = INITIAL_CRC;
        break;
      case HEADER_FRAME_TYPE_POS:
        frame_type = buffer[lpos];
        this->additional_bytes_ = (frame_type == SEPARATOR_BYTE) ? 1 : 0;
        break;
      }
      chk = checksum(buffer + lpos, 1, chk);
      if (use_crc)
        crc = crc16(buffer[lpos], crc);
      lpos++;
    }
    if (fsize < PURE_HEADER_SIZE)
    {
      err = FrameError::FRAME_TOO_SMALL;
      return lpos;
    } 
    if (fsize > MAX_FRAME_SIZE)
    {
      err = FrameError::FRAME_TOO_BIG;
      return lpos;
    }
    this->frame_type_ = frame_type;
    this->use_crc_ = use_crc;
    this->data_size_ = fsize - PURE_HEADER_SIZE;
    this->checksum_ = chk;
    this->crc_ = crc;
    this->status_ = FrameStatus::FRAME_HEADER_ONLY;
    if (lpos == size)
    {
      err = FrameError::HEADER_ONLY;
      return lpos;
    }
  }
  if (this->status_ == FrameStatus::FRAME_HEADER_ONLY)
  {
    uint8_t min_size = this->data_size_ + (this->use_crc_ ? 3 : 1);
    if (size - lpos < min_size)
    {
      err = FrameError::DATA_SIZE_WRONG;
      return lpos;
    }
    size_t rpos = min_size - 1;
    uint16_t crc = INITIAL_CRC;
    for (size_t i = 0; i < data_size_; i++)
      if (buffer[i] == SEPARATOR_BYTE)
        ++this->additional_bytes_;
    if (this->use_crc_)
    {
      uint16_t frame_crc = buffer[rpos] + (buffer[rpos - 1] << 8);
      crc = crc16(buffer, data_size_, crc_);
      if (crc != frame_crc)
      {
        err = FrameError::CRC_WRONG;
        return lpos;
      }
      rpos -= 2;
    }
    uint8_t chk = checksum(buffer + lpos, this->data_size_, this->checksum_) + ((this->additional_bytes_ * SEPARATOR_POST_BYTE) & 0xFF);
    if (chk != buffer[rpos])
    {
      err = FrameError::CHECKSUM_WRONG;
      return lpos;
    }
    this->crc_ = crc;
    this->checksum_ = chk;
    this->status_ = FrameStatus::FRAME_COMPLETE;
    if (this->data_size_ != 0)
    {
      this->data_ = new uint8_t[data_size_];
      memcpy(this->data_, buffer + lpos, this->data_size_);
    }
    else
      this->data_ = nullptr;
    err = FrameError::COMPLETE_FRAME;
    return lpos;
  }
  err = FrameError::UNKNOWN_DATA;
  return lpos;
}

void HaierFrame::reset()
{
  delete[] this->data_;
  this->frame_type_ = 0;
  this->use_crc_ = true;
  this->data_size_ = 0;
  this->checksum_ = 0;
  this->crc_ = INITIAL_CRC;
  this->data_ = nullptr;
  this->status_ = FrameStatus::FRAME_EMPTY;
  this->additional_bytes_ = 0;
}

size_t HaierFrame::get_buffer_size() const
{
  // FF FF <header> <data> <checksum> [CRC]
  size_t result = FRAME_HEADER_SIZE + this->data_size_ + this->additional_bytes_;
  if (this->checksum_ == SEPARATOR_BYTE)
    result += 2;
  else
    result += 1;
  if (this->use_crc_)
  {
    result += 2;
    if ((this->crc_ & 0xFF) == SEPARATOR_BYTE)
      ++result;
    if (((this->crc_ >> 8) & 0xFF) == SEPARATOR_BYTE)
      ++result;
  }
  return  result;
}

size_t HaierFrame::fill_buffer(uint8_t* const buffer, size_t limit) const
{
#define SET_WITH_POST_BYTE(value, dst, position)    do {\
                (dst)[(position)++] = (value); \
                if ((value) == SEPARATOR_BYTE) \
                { \
                    (dst)[(position)++] = SEPARATOR_POST_BYTE; \
                } \
            } while (0)
  size_t sz = get_buffer_size();
  if (sz > limit)
    return 0;
  uint8_t pos = 0;
  for (size_t i = 0; i < FRAME_SEPARATORS_COUNT; i++)
    buffer[pos++] = get_header_byte_(i);
  for (size_t i = FRAME_SEPARATORS_COUNT; i < FRAME_HEADER_SIZE; i++)
  {
    uint8_t val = get_header_byte_(i);
    SET_WITH_POST_BYTE(val, buffer, pos);
  }
  for (size_t i = 0; i < data_size_; i++)
  {
    uint8_t val = this->data_[i];
    SET_WITH_POST_BYTE(val, buffer, pos);
  }
  SET_WITH_POST_BYTE(this->checksum_, buffer, pos);
  if (this->use_crc_)
  {
    uint8_t val = (this->crc_ >> 8) & 0xFF;
    SET_WITH_POST_BYTE(val, buffer, pos);
    val = this->crc_ & 0xFF;
    SET_WITH_POST_BYTE(val, buffer, pos);
  }
  return sz;
#undef SET_WITH_POST_BYTE
}

} // haier_protocol