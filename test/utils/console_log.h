#ifndef CONSOLE_LOG
#define CONSOLE_LOG
#include "Utils/haier_log.h"

void console_logger(HaierProtocol::HaierLogLevel level, const char* tag, const char* format, ...);


#endif // CONSOLE_LOG