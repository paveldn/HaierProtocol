#include <stdint.h>
#include "utils/haier_log.h"
#include "protocol/haier_protocol.h"
#include "console_log.h"
#include "serial_stream.h"  
#include <iostream>
#include <string>

void main(int argc, char** argv) {
  if (argc == 2) {
    HWND console_wnd;
    console_wnd = GetForegroundWindow();
    haier_protocol::set_log_handler(console_logger);
    SerailStream serial_stream(std::string("\\\\.\\").append(argv[1]).c_str());
    if (!serial_stream.is_valid()) {
        std::cout << "Can't open port " << argv[1] << std::endl;
        return;
    }
    haier_protocol::ProtocolHandler smartair2_handler(serial_stream);
    SetConsoleTitle("hOn HVAC simulator, press ESC to exit");
    while ((console_wnd != GetForegroundWindow()) || ((GetKeyState(VK_ESCAPE) & 0x8000) == 0)) {
        smartair2_handler.loop();
        Sleep(50);
    }
	} else {
		std::cout << "Please use: hon_simulator <port>" << std::endl;
	}
}
