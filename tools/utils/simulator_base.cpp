#include "simulator_base.h"
#include "console_log.h"
#include "serial_stream.h"
#include <thread>
#include <iostream>

bool app_exiting{ false };
int last_key_pressed{ 0 };

void protocol_loop(haier_protocol::ProtocolHandler* handler, protocol_preloop ploop) {
  while (!app_exiting) {
    ploop(handler);
    handler->loop();
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
  }
}

void simulator_main(const char* app_name, const char* port_name, message_handlers mhandlers, keyboard_handlers khandlers, protocol_preloop ploop) {
  simulator_main(app_name, port_name, mhandlers, answer_handlers(), khandlers, ploop);
}

void simulator_main(const char* app_name, const char* port_name, message_handlers mhandlers, answer_handlers ahandlers, keyboard_handlers khandlers, protocol_preloop ploop) {
  haier_protocol::set_log_handler(console_logger);
  SerialStream serial_stream(port_name);
  if (!serial_stream.is_valid()) {
    std::cout << "Can't open port " << port_name << std::endl;
    return;
  }
  haier_protocol::ProtocolHandler protocol_handler(serial_stream);
  protocol_handler.set_answer_timeout(1000);
  for (auto it = mhandlers.begin(); it != mhandlers.end(); it++)
    protocol_handler.set_message_handler(it->first, std::bind(it->second, &protocol_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  for (auto it = ahandlers.begin(); it != ahandlers.end(); it++)
    protocol_handler.set_answer_handler(it->first, std::bind(it->second, &protocol_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  std::thread protocol_thread(std::bind(&protocol_loop, &protocol_handler, ploop));
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
