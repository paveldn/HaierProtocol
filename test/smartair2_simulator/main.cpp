#include <stdint.h>
#include "utils/haier_log.h"
#include "protocol/haier_protocol.h"
#include "console_log.h"
#include "serial_stream.h"  
#include "smartair2_packet.h"
#include <iostream>
#include <string>
#include <thread>

using namespace esphome::haier::smartair2_protocol;

HaierPacketControl ac_state;
const haier_protocol::HaierMessage INVALID_MSG((uint8_t)FrameType::INVALID, 0x0000);
bool app_exiting{ false };

void init_ac_state(HaierPacketControl& state) {
  memset(&state, 0, sizeof(HaierPacketControl));
  state.room_temperature = 18;
  state.room_humidity = 56;
  state.cntrl = 0x7F;
  state.ac_mode = (uint8_t) ConditioningMode::AUTO;
  state.fan_mode = (uint8_t) FanMode::FAN_AUTO;
  state.swing_both = 0;
  state.lock_remote = 0;
  state.ac_power = 0;
  state.health_mode = 0;
  state.compressor = 0;
  state.half_degree = 0;
  state.use_swing_bits = 0;
  state.turbo_mode = 0;
  state.sleep_mode = 0;
  state.horizontal_swing = 0;
  state.vertical_swing = 0;
  state.display_status = 0;
  state.set_point = 25 - 16;
}

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler *protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t) FrameType::CONTROL) {
    if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x01)) {
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D01, (uint8_t*) &ac_state, sizeof(HaierPacketControl)));
    } else if ((size > 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x5F)) {
      if (size - 2 != sizeof(HaierPacketControl)) {
        HAIER_LOGW("Wrong control packet size, expected %d, received %d", sizeof(HaierPacketControl), size - 2);
        protocol_handler->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      for (unsigned int i = 0; i < sizeof(HaierPacketControl); i++) {
        if (i == offsetof(HaierPacketControl, cntrl))
          continue;
        uint8_t& cbyte = ((uint8_t*)&ac_state)[i];
        if (cbyte != buffer[2 + i]) {
          HAIER_LOGI("Byte #%d changed 0x%02X => 0x%02X", i, cbyte, buffer[2 + i]);
          cbyte = buffer[2 + i];
        }
      }
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D5F, (uint8_t*)&ac_state, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
  }
}

void protocol_loop(haier_protocol::ProtocolHandler* handler) {
  while (!app_exiting) {
    handler->loop();
    Sleep(3);
  }
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
    init_ac_state(ac_state);
    haier_protocol::ProtocolHandler smartair2_handler(serial_stream);
    smartair2_handler.set_message_handler((uint8_t)FrameType::CONTROL, std::bind(&status_request_handler, &smartair2_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    std::thread protocol_thread(std::bind(&protocol_loop, &smartair2_handler));
    SetConsoleTitle("SmartAir2 HVAC simulator, press ESC to exit");
    while ((console_wnd != GetForegroundWindow()) || ((GetKeyState(VK_ESCAPE) & 0x8000) == 0)) {
        Sleep(50);
    }
    app_exiting = true;
    protocol_thread.join();
  } else {
    std::cout << "Please use: smartair2_simulator <port>" << std::endl;
  }
}
