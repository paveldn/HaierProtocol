#include <cstring>
#include "smartair2_server.h"
#include "smartair2_packet.h"

using namespace esphome::haier::smartair2_protocol;

const uint8_t double_zero_bytes[]{ 0x00, 0x00 };
HaierPacketControl ac_status;
const haier_protocol::HaierMessage INVALID_MSG((uint8_t)FrameType::INVALID, double_zero_bytes, 2);
const haier_protocol::HaierMessage CONFIRM_MSG((uint8_t)FrameType::CONFIRM);


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
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError get_device_version_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t)FrameType::GET_DEVICE_VERSION) {
    if ((size == 0) || (size == 2)) {
      static const uint8_t device_version_info_buf[]{
        'E', '+', '+', '2', '.', '1', '8', '\0', // Device protocol version
        '1', '7', '0', '6', '2', '6', '0', '0', // Device software version
        0xF1, // Encryption type (0xF1 - not supported)
        0x00, 0x00, // Reserved
        '1', '7', '0', '5', '2', '6', '0', '0', // Device hardware version
        0x01, // Communication mode (controller/device communication mode supported)
        'U', '-', 'A', 'C', '\0', '\0', '\0', '\0', // Device name
        0x00, // Reserved
        0x04, 0x5B // Device features (CRC is supported)
      };
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::GET_DEVICE_VERSION_RESPONSE, device_version_info_buf, sizeof(device_version_info_buf)), true);
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

haier_protocol::HandlerError get_device_id_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t)FrameType::GET_DEVICE_ID) {
    if (size == 0) {
      static const uint8_t device_id_buf[] = { 0x20, 0x20, 0x62, 0x84, 0x20, 0xD2, 0x85, 0x34, 0x02, 0x12, 0x71, 0xFB, 0xE0, 0xF4, 0x0D, 0x00,
                                  0x00, 0x00, 0x82, 0x0C, 0xC8, 0x1B, 0xF1, 0x3C, 0x46, 0xAB, 0x92, 0x5B, 0xCE, 0x95, 0x77, 0xC0, // TypeID, 32 bytes binary value (automatically generated when a device is created)
                                  0x04 // Device role (accessory device)
      };
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::GET_DEVICE_ID_RESPONSE, device_id_buf, sizeof(device_id_buf)), true);
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


haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t)FrameType::CONTROL) {
    if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x01)) {
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D01, (uint8_t*)&ac_status, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    }
    else if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x02)) {
      // Power ON
      ac_status.ac_power = 1;
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D02, (uint8_t*)&ac_status, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    }
    else if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x03)) {
      // Power OFF
      ac_status.ac_power = 0;
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D03, (uint8_t*)&ac_status, sizeof(HaierPacketControl)));
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
          HAIER_LOGI("Byte #%d changed 0x%02X => 0x%02X", i, cbyte, buffer[2 + i]);
          cbyte = buffer[2 + i];
        }
      }
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D5F, (uint8_t*)&ac_status, sizeof(HaierPacketControl)));
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

