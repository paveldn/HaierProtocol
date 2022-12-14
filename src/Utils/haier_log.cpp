#include <cstdarg>
#include <stdio.h>
#include "Utils/haier_log.h"

#define BUFFER_SIZE	4096

constexpr char hexmap[] =
	"000102030405060708090A0B0C0D0E0F"
	"101112131415161718191A1B1C1D1E1F"
	"202122232425262728292A2B2C2D2E2F"
	"303132333435363738393A3B3C3D3E3F"
	"404142434445464748494A4B4C4D4E4F"
	"505152535455565758595A5B5C5D5E5F"
	"606162636465666768696A6B6C6D6E6F"
	"707172737475767778797A7B7C7D7E7F"
	"808182838485868788898A8B8C8D8E8F"
	"909192939495969798999A9B9C9D9E9F"
	"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
	"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
	"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
	"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
	"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
	"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";

std::string buf2hex(const uint8_t* message, size_t size)
{

	if (size == 0)
		return "";
	std::string raw(size * 3 - 1, ' ');
	for (size_t i = 0; i < size; ++i) {
		const char* p = hexmap + (message[i] * 2);
		raw[3 * i] = p[0];
		raw[3 * i + 1] = p[1];
	}
	return raw;
}

namespace HaierProtocol
{

char msg_buffer[BUFFER_SIZE];

LogHandler globalLogHandler = nullptr;

size_t logHaier(HaierLogLevel level, const char* tag, const char* format, ...)
{
	size_t res = 0;
	if ((globalLogHandler != nullptr) && (level != HaierLogLevel::llNone))
	{
		va_list args;
		va_start(args, format);
		res = vsnprintf(msg_buffer, BUFFER_SIZE, format, args);
		va_end(args);
		globalLogHandler(level, tag, msg_buffer);
	}
	return res;
}

void setLogHandler(LogHandler handler)
{
	globalLogHandler = handler;
}

void resetLogHandler()
{
	globalLogHandler = nullptr;
}



} // HaierProtocol
