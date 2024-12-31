#include "simulator_base.h"
#include "console_log.h"
#include "serial_stream.h"
#include <thread>
#include <iostream>

bool app_exiting{ false };
int last_key_pressed{ 0 };

void protocol_loop(HaierBaseServer* server, protocol_preloop ploop) {
  while (!app_exiting) {
    ploop(server);
    server->loop();
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
  }
}

void simulator_main(const char* app_name, const char* port_name, HaierBaseServer* server, keyboard_handlers khandlers, protocol_preloop ploop) {
  haier_protocol::set_log_handler(console_logger);
  std::thread protocol_thread(std::bind(&protocol_loop, &server, ploop));
#if _WIN32
  SetConsoleTitle(std::string(app_name).append(", port=").append(port_name).append(". Press ESC to exit").c_str());
#endif
#if _WIN32 || USE_CURSES
  HAIER_LOGI("Starting %s application. Press ESC to exit", app_name);
#else
  HAIER_LOGI("Starting %s application. Press Ctrl+C to exit", app_name);
#endif
  while (!app_exiting) {
    int kb = get_kb_hit();
    if (kb != NO_KB_HIT) {
      if (kb == 27)
        app_exiting = true;
      else {
        auto f = khandlers.find(kb);
        if (f != khandlers.end())
          f->second();
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  protocol_thread.join();
}
