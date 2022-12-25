#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#include "console_log.h"

#define BUFFER_SIZE	4096

void console_logger(haier_protocol::HaierLogLevel level, const char* tag, const char* format, ...)
{
#ifdef _WIN32
    constexpr uint16_t ll2color[] =
    {
        0x07,       // 0llNone
        0x0C,       // llError
        0x0E,       // llWarning
        0x0F,       // llInfo
        0x07,       // not used
        0x07,       // llDebug
        0x08,       // llVerbose
    };
#endif
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
    va_list args;
    va_start(args, format);
    vsnprintf(msg_buffer, BUFFER_SIZE, format, args);
    va_end(args);
#ifdef _WIN32
    static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, ll2color[(uint8_t)level]);
#endif
    std::cout << "[" << ll2tag[(uint8_t)level] << "][" << tag << "]: " << msg_buffer << std::endl;
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, 0x07);
#endif
}
