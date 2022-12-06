#include <cstdarg>
#include <stdio.h>
#include "Utils/haier_log.h"

#define BUFFER_SIZE	4096

std::string buf2hex(const uint8_t* message, size_t size)
{
	constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
								'8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	if (size == 0)
		return "";
	std::string raw(size * 3, ' ');
	for (size_t i = 0; i < size; ++i) {
		if (i > 0)
			raw[3 * i - 1] = ' ';
		raw[3 * i] = hexmap[(message[i] & 0xF0) >> 4];
		raw[3 * i + 1] = hexmap[message[i] & 0x0F];
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
