#include <stdint.h>
#include "utils/haier_log.h"
#include "protocol/haier_protocol.h"
#include "console_log.h"
#include "serial_stream.h"  
#include <iostream>
#include <string>

#define CMD_STATUS_REQUEST 0x01
#define CMD_STATUS_ANSWER 0x02

uint8_t state_buffer[] = { 0x00, 0x1B, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06 };

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler *protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == CMD_STATUS_REQUEST) {
    if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x01)) {
      protocol_handler->send_answer(haier_protocol::HaierMessage(CMD_STATUS_ANSWER, 0x6D01, state_buffer, sizeof(state_buffer)));
    } else if ((size > 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x5F)) {
      Sleep(30); // Simulate answer delay
      memcpy(state_buffer, &buffer[2], min(size - 2, sizeof(state_buffer)));
      protocol_handler->send_answer(haier_protocol::HaierMessage(CMD_STATUS_ANSWER, 0x6D5F, state_buffer, sizeof(state_buffer)));
    } else
      return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
  } else
    return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
}

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
    smartair2_handler.set_message_handler(CMD_STATUS_REQUEST, std::bind(&status_request_handler, &smartair2_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    SetConsoleTitle("SmartAir2 HVAC simulator, press ESC to exit");
    while ((console_wnd != GetForegroundWindow()) || ((GetKeyState(VK_ESCAPE) & 0x8000) == 0)) {
        smartair2_handler.loop();
        Sleep(50);
    }
	} else {
		std::cout << "Please use: smartair2_simulator <port>" << std::endl;
	}
}
