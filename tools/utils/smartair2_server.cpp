#include <cstring>
#include "smartair2_server.h"
#include "smartair2_packet.h"

using namespace esphome::haier::smartair2_protocol;

const uint8_t double_zero_bytes[]{ 0x00, 0x00 };
HaierPacketControl ac_status;
const haier_protocol::HaierMessage INVALID_MSG(haier_protocol::FrameType::INVALID, double_zero_bytes, 2);
const haier_protocol::HaierMessage CONFIRM_MSG(haier_protocol::FrameType::CONFIRM);


void init_ac_state(HaierPacketControl& state) {
  memset(&state, 0, sizeof(HaierPacketControl));
  uint8_t* buf = (uint8_t*)&ac_status;
  state.room_temperature = 18;
  state.room_humidity = 56;
  state.cntrl = 0x7F;
  state.ac_mode = (uint8_t)ConditioningMode::AUTO;
  state.fan_mode = (uint8_t)FanMode::FAN_AUTO;
  state.swing_both = 0;
  state.use_fahrenheit = 0;
  state.lock_remote = 1;
  state.ac_power = 0;
  state.health_mode = 0;
  state.compressor = 1;
  state.ten_degree = 0;
  state.use_swing_bits = 0;
  state.turbo_mode = 1;
  state.quiet_mode = 0;
  state.horizontal_swing = 0;
  state.vertical_swing = 0;
  state.display_status = 0;
  state.set_point = 25 - 16;
}


HaierPacketControl& get_ac_state_ref() {
  static bool _first_run = true;
  if (_first_run) {
    _first_run = false;
    init_ac_state(ac_status);
  }
  return ac_status;
}

haier_protocol::HandlerError report_network_status_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::REPORT_NETWORK_STATUS) {
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
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError unsupported_message_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  HAIER_LOGI("Unsupported message 0x%02X received", type);
  protocol_handler->send_answer(INVALID_MSG);
  return haier_protocol::HandlerError::HANDLER_OK;
}

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::CONTROL) {
    if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x01)) {
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D01, (uint8_t*)&ac_status, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    }
    else if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x02)) {
      // Power ON
      ac_status.ac_power = 1;
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D02, (uint8_t*)&ac_status, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    }
    else if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x03)) {
      // Power OFF
      ac_status.ac_power = 0;
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D03, (uint8_t*)&ac_status, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    }
    else if ((size > 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x5F)) {
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
        uint8_t& cbyte = ((uint8_t*)&ac_status)[i];
        if (cbyte != buffer[2 + i]) {
          HAIER_LOGI("Byte #%d changed 0x%02X => 0x%02X", i + 10, cbyte, buffer[2 + i]);
          cbyte = buffer[2 + i];
        }
      }
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D5F, (uint8_t*)&ac_status, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    }
    else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    }
  }
  else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

