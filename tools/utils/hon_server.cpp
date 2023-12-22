#include <cstring>
#include "hon_server.h"
#include "hon_packet.h"

using namespace esphome::haier::hon_protocol;

const uint8_t double_zero_bytes[]{ 0x00, 0x00 };
HvacFullStatus ac_status;
bool config_mode{ false };
const haier_protocol::HaierMessage INVALID_MSG(haier_protocol::FrameType::INVALID, double_zero_bytes, 2);
const haier_protocol::HaierMessage CONFIRM_MSG(haier_protocol::FrameType::CONFIRM);
uint8_t alarm_status_buf[ALARM_BUF_SIZE] = { 0x00 }; // Alarm mask (no alarms)

uint8_t communication_status{ 0xFF };
std::chrono::steady_clock::time_point last_alarm_message;
bool alarm_paused;
bool alarm_stopped;

constexpr size_t SHORT_ALARM_REPORT_INTERVAL_MS = 300;
constexpr size_t LONG_ALARM_REPORT_INTERVAL_MS = 5000;

bool has_active_alarms() {
  for (int i = 0; i < ALARM_BUF_SIZE; i++)
    if (alarm_status_buf[i] != 0)
      return true;
  return false;
}

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
  state.control.intelligence_status = 0;
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
  state.big_data.power[0] = 0;
  state.big_data.power[0] = 0;
  state.big_data.indoor_coil_temperature = state.sensors.room_temperature + 40;
  state.big_data.outdoor_out_air_temperature = state.sensors.outdoor_temperature;
  state.big_data.outdoor_coil_temperature = state.sensors.outdoor_temperature;
  state.big_data.outdoor_in_air_temperature = state.sensors.outdoor_temperature;
  state.big_data.outdoor_defrost_temperature = state.sensors.outdoor_temperature;
  state.big_data.compressor_frequency = 0;
  state.big_data.compressor_current[0] = 0;
  state.big_data.compressor_current[1] = 0;
  state.big_data.outdoor_fan_status = 0;
  state.big_data.defrost_status = 0;
  state.big_data.compressor_status = 0;
  state.big_data.indoor_fan_status = 0;
  state.big_data.four_way_valve_status = 0;
  state.big_data.indoor_electric_heating_status = 0;
  state.big_data.expansion_valve_open_degree[0] = 0;
  state.big_data.expansion_valve_open_degree[1] = 0;
  last_alarm_message = std::chrono::steady_clock::now();
  alarm_paused = false;
  alarm_stopped = true;
}

bool is_in_configuration_mode() {
  return config_mode;
}

void process_alarms(haier_protocol::ProtocolHandler* protocol_handler)
{
  if (!alarm_stopped && !protocol_handler->is_waiting_for_answer() && (protocol_handler->get_outgoing_queue_size() == 0)) {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (( alarm_paused && (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_alarm_message).count() > LONG_ALARM_REPORT_INTERVAL_MS)) ||
        (!alarm_paused && (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_alarm_message).count() > SHORT_ALARM_REPORT_INTERVAL_MS))) {
      alarm_paused = false;
      protocol_handler->send_message(haier_protocol::HaierMessage(haier_protocol::FrameType::ALARM_STATUS, 0x0F5A, alarm_status_buf, sizeof(alarm_status_buf)), true);
      last_alarm_message = now;
    }
  }
}

HvacFullStatus& get_ac_state_ref() {
  static bool _first_run = true;
  if (_first_run) {
    _first_run = false;
    init_ac_state(ac_status);
  }
  return ac_status;
}

bool start_alarm(uint8_t alarm_id) {
  if (alarm_id >= esphome::haier::hon_protocol::HON_ALARM_COUNT)
    return false;
  alarm_paused = false;
  alarm_stopped = false;
  alarm_status_buf[ALARM_BUF_SIZE - 1 - (uint8_t)(alarm_id / 8)] |= (1 << (alarm_id % 8));
  return true;
}

void reset_alarms() {
  memset(alarm_status_buf, 0, sizeof(ALARM_BUF_SIZE));
  alarm_paused = false;
}

haier_protocol::HandlerError get_device_version_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::GET_DEVICE_VERSION) {
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
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::GET_DEVICE_VERSION_RESPONSE, device_version_info_buf, sizeof(device_version_info_buf)), true);
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError get_device_id_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::GET_DEVICE_ID) {
    if (size == 0) {
      static const uint8_t device_id_buf[] = { 0x20, 0x20, 0x62, 0x84, 0x20, 0xD2, 0x85, 0x34, 0x02, 0x12, 0x71, 0xFB, 0xE0, 0xF4, 0x0D, 0x00,
                                  0x00, 0x00, 0x82, 0x0C, 0xC8, 0x1B, 0xF1, 0x3C, 0x46, 0xAB, 0x92, 0x5B, 0xCE, 0x95, 0x77, 0xC0, // TypeID, 32 bytes binary value (automatically generated when a device is created)
                                  0x04 // Device role (accessory device)
      };
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::GET_DEVICE_ID_RESPONSE, device_id_buf, sizeof(device_id_buf)), true);
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::CONTROL) {
    if (size < 2) {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
    uint16_t subcommand = (buffer[0] << 8) | buffer[1];
    switch (subcommand) {
    case (uint16_t)SubcommandsControl::GET_USER_DATA:
      if (size != 2) {
        protocol_handler->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D01, (uint8_t*)&ac_status, USER_DATA_SIZE));
      return haier_protocol::HandlerError::HANDLER_OK;
    case (uint16_t)SubcommandsControl::GET_BIG_DATA:
      if (size != 2) {
        protocol_handler->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x7D01, (uint8_t*)&ac_status, BIG_DATA_SIZE));
      return haier_protocol::HandlerError::HANDLER_OK;
    case (uint16_t)SubcommandsControl::SET_GROUP_PARAMETERS:
      if (size - 2 != sizeof(HaierPacketControl)) {
        HAIER_LOGW("Wrong control packet size, expected %d, received %d", sizeof(HaierPacketControl), size - 2);
        protocol_handler->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      for (unsigned int i = 0; i < sizeof(HaierPacketControl); i++) {
        uint8_t& cbyte = ((uint8_t*)&ac_status)[i];
        if (cbyte != buffer[2 + i]) {
          HAIER_LOGI("Byte #%d changed 0x%02X => 0x%02X", i + 10, cbyte, buffer[2 + i]);
          cbyte = buffer[2 + i];
        }
      }
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D5F, (uint8_t*)&ac_status, USER_DATA_SIZE));
      return haier_protocol::HandlerError::HANDLER_OK;
    default:
      if ((subcommand & 0xFF00) == (uint16_t)SubcommandsControl::SET_SINGLE_PARAMETER) {
        if (size != 4) {
          HAIER_LOGW("Wrong control packet size, expected 2, received %d", size - 2);
          protocol_handler->send_answer(INVALID_MSG);
          return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
        }
        uint8_t parameter = buffer[1];
        uint16_t value = (buffer[2] << 8) + buffer[3];
        return process_single_parameter(protocol_handler, parameter, value);
      }
      else {
        protocol_handler->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
      }
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError alarm_status_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::GET_ALARM_STATUS) {
    if (size == 0) {
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::GET_ALARM_STATUS_RESPONSE, 0x0F5A, alarm_status_buf, sizeof(alarm_status_buf)));
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError get_management_information_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION) {
    if (size == 0) {
      static const uint8_t management_information_buf[] = { 
        0x00, // Mode switch (normal working condition)
        0x00, // Reserved
        0x00, // Clear user information (no action)
        0x00, // Forced setting (not mandatory)
        0x00, 0x00 // Reserved
      };
      protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION_RESPONSE, management_information_buf, sizeof(management_information_buf)));
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError report_network_status_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::REPORT_NETWORK_STATUS) {
    if (size == 4) {
      uint8_t st = buffer[1];
      config_mode = st == 3;
      if (!config_mode && (ac_status.control.set_point == 0x0E))
        ac_status.control.set_point = 0x0D;
      if (st != communication_status) {
        switch (st) {
        case 0:
          HAIER_LOGI("Network status: Communication is normal");
          break;
        case 1:
          HAIER_LOGI("Network status:  No connection");
          break;
        case 2:
          HAIER_LOGI("Network status:  Server unavailable");
          break;
        case 3:
          HAIER_LOGI("Network status:  Module is in configuration mode");
          break;
        default:
          HAIER_LOGW("Network status:  Unknown status 0x02X", st);
          break;
        }
        communication_status = st;
      }
      protocol_handler->send_answer(CONFIRM_MSG);
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
      protocol_handler->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  } else {
    protocol_handler->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError alarm_status_report_answer_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType request_type, haier_protocol::FrameType message_type, const uint8_t* data, size_t data_size) {
  if (request_type == haier_protocol::FrameType::ALARM_STATUS) {
    if (message_type == haier_protocol::FrameType::CONFIRM) {
      if (data_size == 0) {
        if (!has_active_alarms())
          alarm_stopped = true;
        else
          alarm_paused = true;
        return haier_protocol::HandlerError::HANDLER_OK;
      }
      else
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
    else
      return haier_protocol::HandlerError::INVALID_ANSWER;
  }
  else
    return haier_protocol::HandlerError::UNEXPECTED_MESSAGE;
}

haier_protocol::HandlerError stop_alarm_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::STOP_FAULT_ALARM) {
    if (size == 0) {
      HAIER_LOGI("Stop alarm message received.");
      alarm_stopped = true;
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

haier_protocol::HandlerError process_single_parameter(haier_protocol::ProtocolHandler* protocol_handler, uint8_t parameter, uint16_t value)
{
  #define SET_IF_DIFFERENT(VALUE, FIELD) \
      do { \
        if (ac_status.control.FIELD != VALUE) { \
          HAIER_LOGI(#FIELD" <= %u", VALUE); \
          ac_status.control.FIELD = VALUE; \
        } \
      } while (0)
  haier_protocol::HandlerError result = haier_protocol::HandlerError::HANDLER_OK;
  switch (parameter) {
    case (uint8_t)DataParameters::AC_POWER:
      if (value <= 1)
        SET_IF_DIFFERENT(value, ac_power);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::SET_POINT:
      if ((parameter >= 0) && (parameter <= 14))
        SET_IF_DIFFERENT(value, set_point);
      else 
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::AC_MODE:
      if ((parameter >= 0) && (parameter <= 6))
        SET_IF_DIFFERENT(value, ac_mode);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::FAN_MODE:
      if (((parameter >= 1) && (parameter <= 3)) || (parameter == 5))
        SET_IF_DIFFERENT(value, fan_mode);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::USE_FAHRENHEIT:
      if (value <= 1)
        SET_IF_DIFFERENT(value, use_fahrenheit);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::TEN_DEGREE:
      if (value <= 1) 
        SET_IF_DIFFERENT(value, ten_degree);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::HEALTH_MODE:
      if (value <= 1)
        SET_IF_DIFFERENT(value, health_mode);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::BEEPER_STATUS:
      if (value <= 1) 
        SET_IF_DIFFERENT(value, beeper_status);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::LOCK_REMOTE:
      if (value <= 1) 
        SET_IF_DIFFERENT(value, lock_remote);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::QUIET_MODE:
      if (value <= 1) 
        SET_IF_DIFFERENT(value, quiet_mode);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    case (uint8_t)DataParameters::FAST_MODE:
      if (value <= 1) 
        SET_IF_DIFFERENT(value, fast_mode);
      else
        result = haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
      break;
    default:
      result = haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
      break;
  }
  if (result == haier_protocol::HandlerError::HANDLER_OK) {
    protocol_handler->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D01, (uint8_t*)&ac_status, USER_DATA_SIZE));
  }
  else {
    protocol_handler->send_answer(INVALID_MSG);
  }
  return result;
  #undef SET_IF_DIFFERENT
}
