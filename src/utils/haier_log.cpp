#include <cstdarg>
#include <stdio.h>
#include "utils/haier_log.h"

#ifndef HAIER_LOG_TAG
#define HAIER_LOG_TAG "haier.protocol"
#endif

const char hex_map[] =
    "00" "01" "02" "03" "04" "05" "06" "07" "08" "09" "0A" "0B" "0C" "0D" "0E" "0F"
    "10" "11" "12" "13" "14" "15" "16" "17" "18" "19" "1A" "1B" "1C" "1D" "1E" "1F"
    "20" "21" "22" "23" "24" "25" "26" "27" "28" "29" "2A" "2B" "2C" "2D" "2E" "2F"
    "30" "31" "32" "33" "34" "35" "36" "37" "38" "39" "3A" "3B" "3C" "3D" "3E" "3F"
    "40" "41" "42" "43" "44" "45" "46" "47" "48" "49" "4A" "4B" "4C" "4D" "4E" "4F"
    "50" "51" "52" "53" "54" "55" "56" "57" "58" "59" "5A" "5B" "5C" "5D" "5E" "5F"
    "60" "61" "62" "63" "64" "65" "66" "67" "68" "69" "6A" "6B" "6C" "6D" "6E" "6F"
    "70" "71" "72" "73" "74" "75" "76" "77" "78" "79" "7A" "7B" "7C" "7D" "7E" "7F"
    "80" "81" "82" "83" "84" "85" "86" "87" "88" "89" "8A" "8B" "8C" "8D" "8E" "8F"
    "90" "91" "92" "93" "94" "95" "96" "97" "98" "99" "9A" "9B" "9C" "9D" "9E" "9F"
    "A0" "A1" "A2" "A3" "A4" "A5" "A6" "A7" "A8" "A9" "AA" "AB" "AC" "AD" "AE" "AF"
    "B0" "B1" "B2" "B3" "B4" "B5" "B6" "B7" "B8" "B9" "BA" "BB" "BC" "BD" "BE" "BF"
    "C0" "C1" "C2" "C3" "C4" "C5" "C6" "C7" "C8" "C9" "CA" "CB" "CC" "CD" "CE" "CF"
    "D0" "D1" "D2" "D3" "D4" "D5" "D6" "D7" "D8" "D9" "DA" "DB" "DC" "DD" "DE" "DF"
    "E0" "E1" "E2" "E3" "E4" "E5" "E6" "E7" "E8" "E9" "EA" "EB" "EC" "ED" "EE" "EF"
    "F0" "F1" "F2" "F3" "F4" "F5" "F6" "F7" "F8" "F9" "FA" "FB" "FC" "FD" "FE" "FF";

std::string buf_to_hex(const uint8_t *message, size_t size)
{
  if (size == 0)
    return "";
  std::string raw(size * 3 - 1, ' ');
  for (size_t i = 0; i < size; ++i)
  {
    const char *p = hex_map + (message[i] * 2);
    raw[3 * i] = p[0];
    raw[3 * i + 1] = p[1];
  }
  return raw;
}

size_t print_buf(const uint8_t *src_buf, size_t src_size, char *dst_buf, size_t dst_size)
{
  size_t bytes_to_print = src_size;
  bool dots = false;
  size_t pos = 0;
  if (src_size > 0)
  {
    if (src_size * 3 > dst_size)
    {
      bytes_to_print = (dst_size - 5) / 3;
      dots = true;
    }
    for (size_t i = 0; i < bytes_to_print; i++)
    {
      const char *p = hex_map + (src_buf[i] * 2);
      dst_buf[3 * i] = p[0];
      dst_buf[3 * i + 1] = p[1];
      dst_buf[3 * i + 2] = ' ';
    }
    pos = 3 * bytes_to_print - 1;
    if (dots)
    {
      pos++;
      dst_buf[pos++] = '.';
      dst_buf[pos++] = '.';
      dst_buf[pos++] = '.';
    }
    dst_buf[pos] = '\0';
  }
  return pos;
}

namespace haier_protocol
{

constexpr size_t BUFFER_SIZE = 4096;

char msg_buffer[BUFFER_SIZE];

LogHandler global_log_handler = nullptr;

size_t log_haier(HaierLogLevel level, const char *format, ...)
{
  size_t res = 0;
  if ((global_log_handler != nullptr) && (level != HaierLogLevel::LEVEL_NONE))
  {
    va_list args;
    va_start(args, format);
    res = vsnprintf(msg_buffer, BUFFER_SIZE, format, args);
    va_end(args);
    global_log_handler(level, HAIER_LOG_TAG, msg_buffer);
  }
  return res;
}

size_t log_haier_buffer(HaierLogLevel level, const char *header, const uint8_t *buffer, size_t size)
{
  return log_haier_buffers(level, header, buffer, size, nullptr, 0);
}

size_t log_haier_buffers(HaierLogLevel level, const char *header, const uint8_t *buffer1, size_t size1, const uint8_t *buffer2, size_t size2)
{
  size_t res = 0;
  if ((global_log_handler != nullptr) && (level != HaierLogLevel::LEVEL_NONE))
  {
    if (header != nullptr)
    {
      res = 0;
      while (res <= BUFFER_SIZE - 7)
      {
        if (header[res] == '\0')
          break;
        msg_buffer[res] = header[res];
        ++res;
      }
      msg_buffer[res++] = ' ';
    }
    if (size1 + size2 == 0)
      res += snprintf(msg_buffer + res, BUFFER_SIZE - res - 1, "<empty>");
    else
    {
      if ((buffer1 != nullptr) && (size1 > 0))
      {
        res += print_buf(buffer1, size1, msg_buffer + res, BUFFER_SIZE - res);
        if ((BUFFER_SIZE - res > 0) && (size2 > 0))
        {
          msg_buffer[res++] = ' ';
          res += print_buf(buffer2, size2, msg_buffer + res, BUFFER_SIZE - res);
        }
      }
    }
    global_log_handler(level, HAIER_LOG_TAG, msg_buffer);
  }
  return res;
}

void set_log_handler(LogHandler handler)
{
  global_log_handler = handler;
}

void reset_log_handler()
{
  global_log_handler = nullptr;
}

} // haier_protocol
