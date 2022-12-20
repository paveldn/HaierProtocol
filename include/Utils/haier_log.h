#ifndef HAIER_LOG_H
#define HAIER_LOG_H

#include <functional>
#include <string>

extern const char hexmap[];

#ifndef HAIER_LOG_LEVEL
    #define HAIER_LOG_LEVEL 0
#endif

#if (HAIER_LOG_LEVEL > 0)
    #define HAIER_LOGE(...)	logHaier(HaierProtocol::HaierLogLevel::llError, __VA_ARGS__)
#else
    #define HAIER_LOGE(...)
#endif
#if (HAIER_LOG_LEVEL > 1)
    #define HAIER_LOGW(...)	logHaier(HaierProtocol::HaierLogLevel::llWarning, __VA_ARGS__)
#else
    #define HAIER_LOGW(...)
#endif
#if (HAIER_LOG_LEVEL > 2)
    #define HAIER_LOGI(...)	logHaier(HaierProtocol::HaierLogLevel::llInfo, __VA_ARGS__)
#else
    #define HAIER_LOGI(...)
#endif
#if (HAIER_LOG_LEVEL > 3)
    #define HAIER_LOGD(...)	logHaier(HaierProtocol::HaierLogLevel::llDebug, __VA_ARGS__)
#else
    #define HAIER_LOGD(...)
#endif
#if (HAIER_LOG_LEVEL > 4)
    #define HAIER_LOGV(...)	logHaier(HaierProtocol::HaierLogLevel::llVerbose, __VA_ARGS__)
#else
    #define HAIER_LOGV(...)
#endif

#if (HAIER_LOG_LEVEL > 0)
#define HAIER_BUFE(header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llError, header, buffer, size)
#else
#define HAIER_BUFE(header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 1)
#define HAIER_BUFW(header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llWarning, header, buffer, size)
#else
#define HAIER_BUFW(header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 2)
#define HAIER_BUFI(header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llInfo, header, buffer, size)
#else
#define HAIER_BUFI(header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 3)
#define HAIER_BUFD(header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llDebug, header, buffer, size)
#else
#define HAIER_BUFD(header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 4)
#define HAIER_BUFV(header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llVerbose, header, buffer, size)
#else
#define HAIER_BUFV(header, buffer, size)
#endif

std::string buf2hex(const uint8_t* message, size_t size);

namespace HaierProtocol
{

enum class HaierLogLevel
{
    llNone = 0,
    llError = 1,
    llWarning = 2,
    llInfo = 3,
    llDebug = 5,
    llVerbose = 6
};
                          // <log_level>,       <tag>,   <message>
typedef std::function<void(HaierLogLevel, const char*, const char*)> LogHandler;

size_t logHaier(HaierLogLevel level, const char* format, ...);
size_t logHaierBuffer(HaierLogLevel level, const char* header, const uint8_t* buffer, size_t size);
void setLogHandler(LogHandler);
void resetLogHandler();

}
#endif // HAIER_LOG_H

