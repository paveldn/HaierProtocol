#include <stdint.h>
#include "simulator_base.h"
#include "smartair2_packet.h"
#include <string>
#include <iostream>

using namespace esphome::haier::smartair2_protocol;

HaierPacketControl ac_state;
const haier_protocol::HaierMessage INVALID_MSG((uint8_t)FrameType::INVALID, 0x0000);
const haier_protocol::HaierMessage CONFIRM_MSG((uint8_t)FrameType::CONFIRM);
bool toggle_ac_power{ false };

void init_ac_state(HaierPacketControl& state) {
  memset(&state, 0, sizeof(HaierPacketControl));
  uint8_t* buf = (uint8_t*)&ac_state;
  state.room_temperature = 18;
  state.room_humidity = 56;
  state.cntrl = 0x7F;
  state.ac_mode = (uint8_t) ConditioningMode::AUTO;
  state.fan_mode = (uint8_t) FanMode::FAN_AUTO;
  state.swing_both = 0;
  state.use_fahrenheit = 0;
  state.lock_remote = 1;
  state.ac_power = 0;
  state.health_mode = 0;
  state.compressor = 1;
  state.ten_degree = 0;
  state.use_swing_bits = 0;
  state.turbo_mode = 1;
  state.sleep_mode = 0;
  state.horizontal_swing = 0;
  state.vertical_swing = 0;
  state.display_status = 0;
  state.set_point = 25 - 16;
}

haier_protocol::HandlerError report_network_status_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t)FrameType::REPORT_NETWORK_STATUS) {
    if (size == 4) {
      protocol_handler->send_answer(CONFIRM_MSG);
      return haier_protocol::HandlerError::HANDLER_OK;
    }
    else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  }
  else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError get_device_version_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  // Always answer invalid message
  protocol_handler->send_answer(INVALID_MSG);
  return haier_protocol::HandlerError::HANDLER_OK;
}

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler *protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t) FrameType::CONTROL) {
    if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x01)) {
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D01, (uint8_t*)&ac_state, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    } else if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x02)) {
      // Power ON
      ac_state.ac_power = 1;
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D02, (uint8_t*)&ac_state, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    } else if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x03)) {
      // Power OFF
      ac_state.ac_power = 0;
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D03, (uint8_t*)&ac_state, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    } else if ((size > 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x5F)) {
      if (size - 2 != sizeof(HaierPacketControl)) {
        HAIER_LOGW("Wrong control packet size, expected %d, received %d", sizeof(HaierPacketControl), size - 2);
        protocol_handler->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      for (unsigned int i = 0; i < sizeof(HaierPacketControl); i++) {
        if ((i == offsetof(HaierPacketControl, cntrl)) ||
            (i == offsetof(HaierPacketControl, room_temperature)) ||
            (i == offsetof(HaierPacketControl, room_humidity)))
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

void preloop(haier_protocol::ProtocolHandler* handler) {
  if (toggle_ac_power) {
    toggle_ac_power = false;
    uint8_t ac_power = ac_state.ac_power;
    ac_state.ac_power = ac_power == 1 ? 0 : 1;
    HAIER_LOGI("AC power is %s", ac_power == 1 ? "Off" : "On");
  }
}

void main(int argc, char** argv) {
  if (argc == 2) {
    init_ac_state(ac_state);
    message_handlers mhandlers;
    mhandlers[(uint8_t) FrameType::CONTROL] = status_request_handler;
    mhandlers[(uint8_t) FrameType::REPORT_NETWORK_STATUS] = report_network_status_handler;
    keyboard_handlers khandlers;
    khandlers['1'] = []() { toggle_ac_power = true; };
    simulator_main("SmartAir2 HVAC simulator", argv[1], mhandlers, khandlers, preloop);
  } else {
    std::cout << "Please use: smartair2_simulator <port>" << std::endl;
  }
}
