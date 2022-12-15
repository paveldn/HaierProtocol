#ifndef HAIER_LOG_H
#define HAIER_LOG_H

#include <functional>
#include <string>

extern const char hexmap[];

#ifndef HAIER_LOG_LEVEL
    #ifdef ESPHOME_LOG_LEVEL
        #if (ESPHOME_LOG_LEVEL == "NONE") 
            #define HAIER_LOG_LEVEL 0
        #elif (ESPHOME_LOG_LEVEL == "ERROR") 
            #define HAIER_LOG_LEVEL 1
        #elif (ESPHOME_LOG_LEVEL == "WARN") 
            #define HAIER_LOG_LEVEL 2
        #elif (ESPHOME_LOG_LEVEL == "INFO") 
            #define HAIER_LOG_LEVEL 3
        #elif (ESPHOME_LOG_LEVEL == "DEBUG") 
            #define HAIER_LOG_LEVEL 4
        #elif (ESPHOME_LOG_LEVEL == "VERBOSE") 
            #define HAIER_LOG_LEVEL 5
        #elif (ESPHOME_LOG_LEVEL == "VERY_VERBOSE") 
            #define HAIER_LOG_LEVEL 6
        #else
            #error "Unknown ESPHome log level!"
        #endif
    #else
        #define HAIER_LOG_LEVEL 0
    #endif
#endif

#if (HAIER_LOG_LEVEL > 0)
    #define HAIER_LOGE(tag, ...)	logHaier(HaierProtocol::HaierLogLevel::llError, tag, __VA_ARGS__)
#else
    #define HAIER_LOGE(tag, ...)
#endif
#if (HAIER_LOG_LEVEL > 1)
    #define HAIER_LOGW(tag, ...)	logHaier(HaierProtocol::HaierLogLevel::llWarning, tag, __VA_ARGS__)
#else
    #define HAIER_LOGW(tag, ...)
#endif
#if (HAIER_LOG_LEVEL > 2)
    #define HAIER_LOGI(tag, ...)	logHaier(HaierProtocol::HaierLogLevel::llInfo, tag, __VA_ARGS__)
#else
    #define HAIER_LOGI(tag, ...)
#endif
#if (HAIER_LOG_LEVEL > 3)
    #define HAIER_LOGD(tag, ...)	logHaier(HaierProtocol::HaierLogLevel::llDebug, tag, __VA_ARGS__)
#else
    #define HAIER_LOGD(tag, ...)
#endif
#if (HAIER_LOG_LEVEL > 4)
    #define HAIER_LOGV(tag, ...)	logHaier(HaierProtocol::HaierLogLevel::llVerbose, tag, __VA_ARGS__)
#else
    #define HAIER_LOGV(tag, ...)
#endif

#if (HAIER_LOG_LEVEL > 0)
#define HAIER_BUFE(tag, header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llError, tag, header, buffer, size)
#else
#define HAIER_BUFE(tag, header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 1)
#define HAIER_BUFW(tag, header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llWarning, tag, header, buffer, size)
#else
#define HAIER_BUFW(tag, header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 2)
#define HAIER_BUFI(tag, header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llInfo, tag, header, buffer, size)
#else
#define HAIER_BUFI(tag, header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 3)
#define HAIER_BUFD(tag, header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llDebug, tag, header, buffer, size)
#else
#define HAIER_BUFD(tag, header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 4)
#define HAIER_BUFV(tag, header, buffer, size)	logHaierBuffer(HaierProtocol::HaierLogLevel::llVerbose, tag, header, buffer, size)
#else
#define HAIER_BUFV(tag, header, buffer, size)
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

size_t logHaier(HaierLogLevel level, const char* tag, const char* format, ...);
size_t logHaierBuffer(HaierLogLevel level, const char* tag, const char* header, const uint8_t* buffer, size_t size);
void setLogHandler(LogHandler);
void resetLogHandler();

}
#endif // HAIER_LOG_H

