#ifndef HAIER_LOG_H
#define HAIER_LOG_H

#include <functional>
#include <string>

#define HAIER_LOGE(tag, ...)	logHaier(HaierProtocol::LogLevel::llError, tag, __VA_ARGS__)
#define HAIER_LOGW(tag, ...)	logHaier(HaierProtocol::LogLevel::llWarning, tag, __VA_ARGS__)
#define HAIER_LOGI(tag, ...)	logHaier(HaierProtocol::LogLevel::llInfo, tag, __VA_ARGS__)
#define HAIER_LOGD(tag, ...)	logHaier(HaierProtocol::LogLevel::llDebug, tag, __VA_ARGS__)
#define HAIER_LOGV(tag, ...)	logHaier(HaierProtocol::LogLevel::llVerbose, tag, __VA_ARGS__)

std::string buf2hex(const uint8_t* message, size_t size);

namespace HaierProtocol
{

enum class LogLevel
{
	llNone = 0,
	llError = 1,
	llWarning = 2,
	llInfo = 3,
	llDebug = 5,
	llVerbose = 6
};
					// <log_level>,    <tag>,    <message>
typedef std::function<void(LogLevel, const char*, const char*)> LogHandler;

size_t logHaier(LogLevel level, const char* tag, const char* format, ...);
void setLogHandler(LogHandler);
void resetLogHandler();

}
#endif // HAIER_LOG_H

