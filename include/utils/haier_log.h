#ifndef HAIER_LOG_H
#define HAIER_LOG_H

#include <functional>
#include <string>
#include <stdint.h> 

extern const char hex_map[];

#ifndef HAIER_LOG_LEVEL
    #define HAIER_LOG_LEVEL 0
#endif

#if (HAIER_LOG_LEVEL > 0)
    #define HAIER_LOGE(...)	log_haier(haier_protocol::HaierLogLevel::LEVEL_ERROR, __VA_ARGS__)
    #define HAIER_BUFE(header, buffer, size)	log_haier_buffer(haier_protocol::HaierLogLevel::LEVEL_ERROR, header, buffer, size)
#else
    #define HAIER_LOGE(...)
    #define HAIER_BUFE(header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 1)
    #define HAIER_LOGW(...)	log_haier(haier_protocol::HaierLogLevel::LEVEL_WARNING, __VA_ARGS__)
    #define HAIER_BUFW(header, buffer, size)	log_haier_buffer(haier_protocol::HaierLogLevel::LEVEL_WARNING, header, buffer, size)
#else
    #define HAIER_LOGW(...)
    #define HAIER_BUFW(header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 2)
    #define HAIER_LOGI(...)	log_haier(haier_protocol::HaierLogLevel::LEVEL_INFO, __VA_ARGS__)
    #define HAIER_BUFI(header, buffer, size)	log_haier_buffer(haier_protocol::HaierLogLevel::LEVEL_INFO, header, buffer, size)
#else
    #define HAIER_LOGI(...)
    #define HAIER_BUFI(header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 3)
    #define HAIER_LOGD(...)	log_haier(haier_protocol::HaierLogLevel::LEVEL_DEBUG, __VA_ARGS__)
    #define HAIER_BUFD(header, buffer, size)	log_haier_buffer(haier_protocol::HaierLogLevel::LEVEL_DEBUG, header, buffer, size)
#else
    #define HAIER_LOGD(...)
    #define HAIER_BUFD(header, buffer, size)
#endif
#if (HAIER_LOG_LEVEL > 4)
    #define HAIER_LOGV(...)	log_haier(haier_protocol::HaierLogLevel::LEVEL_VERBOSE, __VA_ARGS__)
    #define HAIER_BUFV(header, buffer, size)	log_haier_buffer(haier_protocol::HaierLogLevel::LEVEL_VERBOSE, header, buffer, size)
#else
    #define HAIER_LOGV(...)
    #define HAIER_BUFV(header, buffer, size)
#endif

std::string buf_to_hex(const uint8_t* message, size_t size);

namespace haier_protocol
{

enum class HaierLogLevel
{
    LEVEL_NONE = 0,
    LEVEL_ERROR = 1,
    LEVEL_WARNING = 2,
    LEVEL_INFO = 3,
    LEVEL_DEBUG = 5,
    LEVEL_VERBOSE = 6
};
                                   // <log_level>,       <tag>,   <message>
using LogHandler = std::function<void(HaierLogLevel, const char*, const char*)>;

size_t log_haier(HaierLogLevel level, const char* format, ...);
size_t log_haier_buffer(HaierLogLevel level, const char* header, const uint8_t* buffer, size_t size);
size_t log_haier_buffers(HaierLogLevel level, const char* header, const uint8_t* buffer1, size_t size1, const uint8_t* buffer2, size_t size2);
void set_log_handler(LogHandler);
void reset_log_handler();

}
#endif // HAIER_LOG_H

