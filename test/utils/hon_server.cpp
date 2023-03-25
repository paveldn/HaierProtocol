﻿#include <cstring>
#include "hon_server.h"
#include "hon_packet.h"

using namespace esphome::haier::hon_protocol;

const uint8_t double_zero_bytes[]{ 0x00, 0x00 };
HvacFullStatus ac_status;
const haier_protocol::HaierMessage INVALID_MSG((uint8_t)FrameType::INVALID, double_zero_bytes, 2);
const haier_protocol::HaierMessage CONFIRM_MSG((uint8_t)FrameType::CONFIRM);
uint8_t alarm_status_buf[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // Alarm mask (no alarms)
};

void init_ac_state(HvacFullStatus& state) {
  memset(&state, 0, sizeof(HvacFullStatus));
  state.control.set_point = 25 - 16;
  state.control.vertical_swing_mode = (uint8_t)VerticalSwingMode::AUTO;
  state.control.fan_mode = (uint8_t)FanMode::FAN_AUTO;
  state.control.special_mode = (uint8_t)SpecialMode::NONE;
  state.control.ac_mode = (uint8_t)ConditioningMode::AUTO;
  state.control.ten_degree = 0;
  state.control.display_status = 1;
  state.control.half_degree = 0;
  state.control.intelegence_status = 0;
  state.control.pmv_status = 0;
  state.control.use_fahrenheit = 0;
  state.control.ac_power = 0;
  state.control.health_mode = 0;
  state.control.electric_heating_status = 0;
  state.control.fast_mode = 0;
  state.control.quiet_mode = 0;
  state.control.sleep_mode = 0;
  state.control.lock_remote = 0;
  state.control.beeper_status = 0;
  state.control.target_humidity = 0;
  state.control.horizontal_swing_mode = (uint8_t)HorizontalSwingMode::AUTO;
  state.control.human_sensing_status = 0;
  state.control.change_filter = 0;
  state.control.fresh_air_status = 0;
  state.control.humidification_status = 0;
  state.control.pm2p5_cleaning_status = 0;
  state.control.ch2o_cleaning_status = 0;
  state.control.self_cleaning_status = 0;
  state.control.light_status = 1;
  state.control.energy_saving_status = 0;
  state.control.cleaning_time_status = 0;
  state.sensors.room_temperature = 18 * 2;
  state.sensors.room_humidity = 0;
  state.sensors.outdoor_temperature = 10 + 64;
  state.sensors.pm2p5_level = 0;
  state.sensors.air_quality = 0;
  state.sensors.human_sensing = 0;
  state.sensors.ac_type = 0;
  state.sensors.error_status = 0;
  state.sensors.operation_source = 3;
  state.sensors.operation_mode_hk = 0;
  state.sensors.err_confirmation = 0;
  state.sensors.total_cleaning_time = 0;
  state.sensors.indoor_pm2p5_value = 0;
  state.sensors.outdoor_pm2p5_value = 0;
  state.sensors.ch2o_value = 0;
  state.sensors.voc_value = 0;
  state.sensors.co2_value = 0;
}

HvacFullStatus& get_ac_state_ref() {
  static bool _first_run = true;
  if (_first_run) {
    _first_run = false;
    init_ac_state(ac_status);
  }
  return ac_status;
}

void trigger_random_alarm() {
  size_t range = sizeof(alarm_status_buf) * 8;
  size_t r = std::rand() % range;
  size_t index = r / 8;
  alarm_status_buf[index] |= 1 << (r % 8);
}

void reset_alarms() {
  memset(alarm_status_buf, 0, sizeof(alarm_status_buf));
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
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
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
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t)FrameType::CONTROL) {
    if (size < 2) {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
    uint16_t subcommand = (buffer[0] << 8) | buffer[1];
    switch (subcommand) {
    case (uint16_t)SubcomandsControl::GET_USER_DATA:
      if (size != 2) {
        protocol_handler->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D01, (uint8_t*)&ac_status, sizeof(HvacFullStatus)));
      return haier_protocol::HandlerError::HANDLER_OK;
    case (uint16_t)SubcomandsControl::GET_BIG_DATA:
      if (size != 2) {
        protocol_handler->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D01, (uint8_t*)&ac_status, sizeof(HvacFullStatus)));
      return haier_protocol::HandlerError::HANDLER_OK;
    case (uint16_t)SubcomandsControl::SET_GROUP_PARAMETERS:
      if (size - 2 != sizeof(HaierPacketControl)) {
        HAIER_LOGW("Wrong control packet size, expected %d, received %d", sizeof(HaierPacketControl), size - 2);
        protocol_handler->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      for (unsigned int i = 0; i < sizeof(HaierPacketControl); i++) {
        uint8_t& cbyte = ((uint8_t*)&ac_status)[i];
        if (cbyte != buffer[2 + i]) {
          HAIER_LOGI("Byte #%d changed 0x%02X => 0x%02X", i, cbyte, buffer[2 + i]);
          cbyte = buffer[2 + i];
        }
      }
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::STATUS, 0x6D5F, (uint8_t*)&ac_status, sizeof(HaierPacketControl)));
      return haier_protocol::HandlerError::HANDLER_OK;
    default:
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError alarm_status_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t)FrameType::GET_ALARM_STATUS) {
    if (size == 0) {
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::GET_ALARM_STATUS_RESPONSE, 0x0F5A, alarm_status_buf, sizeof(alarm_status_buf)));
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError get_managment_information_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t)FrameType::GET_MANAGEMENT_INFORMATION) {
    if (size == 0) {
      static const uint8_t managment_information_buf[] = { 
        0x00, // Mode switch (normal working condition)
        0x00, // Reserved
        0x00, // Clear user information (no action)
        0x00, // Forced setting (not mandatory)
        0x00, 0x00 // Reserved
      };
      protocol_handler->send_answer(haier_protocol::HaierMessage((uint8_t)FrameType::GET_MANAGEMENT_INFORMATION_RESPONSE, managment_information_buf, sizeof(managment_information_buf)));
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

haier_protocol::HandlerError report_network_status_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
  if (type == (uint8_t)FrameType::REPORT_NETWORK_STATUS) {
    if (size == 4) {
      protocol_handler->send_answer(CONFIRM_MSG);
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
  }
}