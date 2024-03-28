#ifndef CONSOLE_LOG
#define CONSOLE_LOG
#include "utils/haier_log.h"

#if USE_CURSES
void init_console();

void exit_console();
#endif

constexpr int NO_KB_HIT = -1;

int get_kb_hit();

unsigned int get_warnings_count();

unsigned int get_errors_count();

void console_logger(haier_protocol::HaierLogLevel level, const char* tag, const char* format, ...);


#endif // CONSOLE_LOG