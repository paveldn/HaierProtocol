#include <iostream>
#include <chrono>
#include <cstdarg>
#if _WIN32
#include <windows.h>
#include <conio.h>
#endif
#if USE_CURSES
#include <curses.h>
#include <thread>
#endif
#include "console_log.h"

#define BUFFER_SIZE 4096

// Test counters
unsigned int warnings_counter{ 0 };
unsigned int errors_counter{ 0 };

unsigned int get_warnings_count() {
    return warnings_counter;
}

unsigned int get_errors_count() {
    return errors_counter;
}

#if USE_CURSES
void init_console() {
  initscr();
  start_color();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(6, COLOR_WHITE, COLOR_BLACK);
}

void exit_console() {
  refresh();
#if PRESS_KEY_TO_CLOSE_CONSOLE  
  HAIER_LOGI("Press any key to close console");
  while (get_kb_hit() == NO_KB_HIT)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif    
  echo();
  endwin();
}

class __CONSOLE_GUARD__ {
public:
    __CONSOLE_GUARD__() {
        init_console();
    }
    ~__CONSOLE_GUARD__() {
        exit_console();
    }
};

__CONSOLE_GUARD__ __GUARD__;
#endif

int get_kb_hit() {
  int result = NO_KB_HIT;
#if _WIN32
  if (kbhit())
    result = getch();
#elif USE_CURSES
  int ch = getch();
  if (ch != ERR)
    result = ch;
#endif
  return result;
}

void console_logger(haier_protocol::HaierLogLevel level, const char* tag, const char* format, ...)
{
    constexpr char ll2tag[] =
    {
        '#',        // llNone
        'E',        // llError
        'W',        // llWarning
        'I',        // llInfo
        '#',        // not used
        'D',        // llDebug
        'V',        // llVerbose
    };
    static char msg_buffer[BUFFER_SIZE];
    if (level == haier_protocol::HaierLogLevel::LEVEL_NONE)
        return;
    if (level == haier_protocol::HaierLogLevel::LEVEL_WARNING)
        warnings_counter++;
    if (level == haier_protocol::HaierLogLevel::LEVEL_ERROR)
        errors_counter++;
    std::time_t now = std::time(nullptr);
    std::tm local_now = *std::localtime(&now);
    int len = snprintf(msg_buffer, BUFFER_SIZE, "[%02d:%02d:%02d][%c][%s]: ", local_now.tm_hour, local_now.tm_min, local_now.tm_sec, ll2tag[(uint8_t)level], tag);
    if (len < 0) {
      std::cout << "error writing message!" << std::endl;
      return;
    }
    va_list args;
    va_start(args, format);
    vsnprintf(msg_buffer + len, BUFFER_SIZE - len - 1, format, args);
    va_end(args);
#if USE_CURSES
    attron(COLOR_PAIR((short)level));
    printw("%s\n", msg_buffer);
#else
    const std::string ll2color[] =
    {
      "\033[0m",    // llNone
      "\033[91m",   // llError
      "\033[93m",   // llWarning
      "\033[32m",   // llInfo
      "\033[0m",    // not used
      "\033[37m",   // llDebug
      "\033[90m",   // llVerbose
    };
    std::cout << ll2color[(uint8_t)level] << msg_buffer << "\033[0m" << std::endl;
#endif
#if _WIN32
    // DebugView++ message sending
    HWND debugviewpp_window = FindWindowA(NULL, "[Capture Win32 & Global Win32 Messages] - DebugView++");
    if (debugviewpp_window == NULL)
      debugviewpp_window = FindWindowA(NULL, "[Capture Win32] - DebugView++");
    if (debugviewpp_window != NULL) {
      static unsigned long process_id = GetCurrentProcessId();
      SendMessageA(debugviewpp_window, EM_REPLACESEL, process_id, (LPARAM)msg_buffer);
    }
#endif
}
