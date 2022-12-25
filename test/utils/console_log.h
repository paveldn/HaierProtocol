#ifndef CONSOLE_LOG
#define CONSOLE_LOG
#include "utils/haier_log.h"

void console_logger(haier_protocol::HaierLogLevel level, const char* tag, const char* format, ...);


#endif // CONSOLE_LOG