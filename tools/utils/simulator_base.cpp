#include "simulator_base.h"
#include "console_log.h"
#include "serial_stream.h"
#include <thread>
#include <iostream>
#include <conio.h>

bool app_exiting{ false };

void protocol_loop(haier_protocol::ProtocolHandler* handler, protocol_preloop ploop) {
  while (!app_exiting) {
    ploop(handler);
    handler->loop();
    Sleep(3);
  }
}

void simulator_main(const char* app_name, const char* port_name, message_handlers mhandlers, keyboard_handlers khandlers, protocol_preloop ploop) {
  haier_protocol::set_log_handler(console_logger);
  SerailStream serial_stream(std::string("\\\\.\\").append(port_name).c_str());
  if (!serial_stream.is_valid()) {
    std::cout << "Can't open port " << port_name << std::endl;
    return;
  }
  haier_protocol::ProtocolHandler protocol_handler(serial_stream);
  for (auto it = mhandlers.begin(); it != mhandlers.end(); it++)
    protocol_handler.set_message_handler(it->first, std::bind(it->second, &protocol_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  std::thread protocol_thread(std::bind(&protocol_loop, &protocol_handler, ploop));
  SetConsoleTitle(std::string(app_name).append(", press ESC to exit").c_str());
  while (!app_exiting) {
    if (kbhit()) {
      char ch = getch();
      if (ch == 27)
        app_exiting = true;
      else {
        auto f = khandlers.find(ch);
        if (f != khandlers.end())
          f->second();
      }
    }
    Sleep(50);
  }
  protocol_thread.join();
}
