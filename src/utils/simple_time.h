#ifndef HAIER_SIMPLE_TIME_H
#define HAIER_SIMPLE_TIME_H

#include <stdint.h>

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <chrono>
#endif

namespace simple_time {
using ms_t = uint32_t;

inline ms_t now_ms() noexcept {
#if defined(ARDUINO)
  return static_cast<ms_t>(millis());
#else
  using namespace std::chrono;
  return static_cast<ms_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
#endif
}

inline constexpr ms_t zero_ms() noexcept { return static_cast<ms_t>(0); }

} // namespace simple_time

#endif // HAIER_SIMPLE_TIME_H
