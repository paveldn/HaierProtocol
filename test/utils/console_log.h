#ifndef CONSOLE_LOG
#define CONSOLE_LOG
#include "utils/haier_log.h"

unsigned int get_warnings_counter();

unsigned int get_errors_counter();

void console_logger(haier_protocol::HaierLogLevel level, const char* tag, const char* format, ...);


#endif // CONSOLE_LOG