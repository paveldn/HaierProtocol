#include <cstring>
#include "hon_server.h"
#include "hon_packet.h"
#include "simulator_base.h"
#include <cassert>

using namespace esphome::haier::hon_protocol;

const uint8_t double_zero_bytes[]{ 0x00, 0x00 };

const haier_protocol::HaierMessage INVALID_MSG(haier_protocol::FrameType::INVALID, double_zero_bytes, 2);
const haier_protocol::HaierMessage CONFIRM_MSG(haier_protocol::FrameType::CONFIRM);

constexpr size_t SHORT_ALARM_REPORT_INTERVAL_MS = 300;
constexpr size_t LONG_ALARM_REPORT_INTERVAL_MS = 5000;

HonServer::HonServer(haier_protocol::ProtocolStream& stream, HonProtocolSettings settings) : 
    protocol_settings_(settings),
	status_message_buffer_size_(settings.get_total_status_data_size()),
	status_message_buffer_(new uint8_t[status_message_buffer_size_]),
	protocol_handler_(new haier_protocol::ProtocolHandler(stream)) 
{
	assert(settings.control_packet_size >= sizeof(esphome::haier::hon_protocol::HaierPacketControl));
	assert(settings.sensors_packet_size >= sizeof(esphome::haier::hon_protocol::HaierPacketSensors));
	assert(settings.big_data_packet_size >= sizeof(esphome::haier::hon_protocol::HaierPacketBigData));
	this->init_ac_state_internal_();
    this->register_handlers_();
}

HonServer::HonServer(haier_protocol::ProtocolStream& stream) : HonServer(stream, HonProtocolSettings()) {
}

HonServer::~HonServer() {
    delete this->protocol_handler_;
}

void HonServer::register_handlers_() {
    this->protocol_handler_->set_message_handler(haier_protocol::FrameType::GET_DEVICE_VERSION, std::bind(&HonServer::get_device_version_handler_, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    this->protocol_handler_->set_message_handler(haier_protocol::FrameType::GET_DEVICE_ID, std::bind(&HonServer::get_device_id_handler_, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    this->protocol_handler_->set_message_handler(haier_protocol::FrameType::CONTROL, std::bind(&HonServer::status_request_handler_, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    this->protocol_handler_->set_message_handler(haier_protocol::FrameType::GET_ALARM_STATUS, std::bind(&HonServer::alarm_status_handler_, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	this->protocol_handler_->set_message_handler(haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION, std::bind(&HonServer::get_management_information_handler_, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	this->protocol_handler_->set_message_handler(haier_protocol::FrameType::REPORT_NETWORK_STATUS, std::bind(&HonServer::report_network_status_handler_, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HonServer::init_ac_state_internal_() {
    memset(this->status_message_buffer_, 0, this->status_message_buffer_size_);
    this->control_packet_ = (esphome::haier::hon_protocol::HaierPacketControl*)&this->status_message_buffer_[this->protocol_settings_.status_message_header_size];
    this->sensors_packet_ = (esphome::haier::hon_protocol::HaierPacketSensors*)&this->status_message_buffer_[this->protocol_settings_.status_message_header_size + this->protocol_settings_.control_packet_size];
    this->big_data_packet_ = (esphome::haier::hon_protocol::HaierPacketBigData*)&this->status_message_buffer_[this->protocol_settings_.status_message_header_size + this->protocol_settings_.control_packet_size + this->protocol_settings_.sensors_packet_size];
    memset(this->alarm_status_buf_, 0, ALARM_BUF_SIZE); // No alarms
    this->control_packet_->set_point = 25 - 16;
    this->control_packet_->vertical_swing_mode = (uint8_t)VerticalSwingMode::AUTO;
    this->control_packet_->fan_mode = (uint8_t)FanMode::FAN_AUTO;
    this->control_packet_->special_mode = (uint8_t)SpecialMode::NONE;
    this->control_packet_->ac_mode = (uint8_t)ConditioningMode::AUTO;
    this->control_packet_->ten_degree = 0;
    this->control_packet_->display_status = 1;
    this->control_packet_->half_degree = 0;
    this->control_packet_->intelligence_status = 0;
    this->control_packet_->pmv_status = 0;
    this->control_packet_->use_fahrenheit = 0;
    this->control_packet_->ac_power = 0;
    this->control_packet_->health_mode = 0;
    this->control_packet_->electric_heating_status = 0;
    this->control_packet_->fast_mode = 0;
    this->control_packet_->quiet_mode = 0;
    this->control_packet_->sleep_mode = 0;
    this->control_packet_->lock_remote = 0;
    this->control_packet_->beeper_status = 0;
    this->control_packet_->target_humidity = 0;
    this->control_packet_->horizontal_swing_mode = (uint8_t)HorizontalSwingMode::AUTO;
    this->control_packet_->human_sensing_status = 0;
    this->control_packet_->change_filter = 0;
    this->control_packet_->fresh_air_status = 0;
    this->control_packet_->humidification_status = 0;
    this->control_packet_->pm2p5_cleaning_status = 0;
    this->control_packet_->ch2o_cleaning_status = 0;
    this->control_packet_->self_cleaning_status = 0;
    this->control_packet_->light_status = 1;
    this->control_packet_->energy_saving_status = 0;
    this->control_packet_->cleaning_time_status = 0;
    this->sensors_packet_->room_temperature = 18 * 2;
    this->sensors_packet_->room_humidity = 0;
    this->sensors_packet_->outdoor_temperature = 10 + 64;
    this->sensors_packet_->pm2p5_level = 0;
    this->sensors_packet_->air_quality = 0;
    this->sensors_packet_->human_sensing = 0;
    this->sensors_packet_->ac_type = 0;
    this->sensors_packet_->error_status = 0;
    this->sensors_packet_->operation_source = 3;
    this->sensors_packet_->operation_mode_hk = 0;
    this->sensors_packet_->err_confirmation = 0;
    this->sensors_packet_->total_cleaning_time = 0;
    this->sensors_packet_->indoor_pm2p5_value = 0;
    this->sensors_packet_->outdoor_pm2p5_value = 0;
    this->sensors_packet_->ch2o_value = 0;
    this->sensors_packet_->voc_value = 0;
    this->sensors_packet_->co2_value = 0;
    this->big_data_packet_->power[0] = 0;
    this->big_data_packet_->power[0] = 0;
    this->big_data_packet_->indoor_coil_temperature = this->sensors_packet_->room_temperature + 40;
    this->big_data_packet_->outdoor_out_air_temperature = this->sensors_packet_->outdoor_temperature;
    this->big_data_packet_->outdoor_coil_temperature = this->sensors_packet_->outdoor_temperature;
    this->big_data_packet_->outdoor_in_air_temperature = this->sensors_packet_->outdoor_temperature;
    this->big_data_packet_->outdoor_defrost_temperature = this->sensors_packet_->outdoor_temperature;
    this->big_data_packet_->compressor_frequency = 0;
    this->big_data_packet_->compressor_current[0] = 0;
    this->big_data_packet_->compressor_current[1] = 0;
    this->big_data_packet_->outdoor_fan_status = 0;
    this->big_data_packet_->defrost_status = 0;
    this->big_data_packet_->compressor_status = 0;
    this->big_data_packet_->indoor_fan_status = 0;
    this->big_data_packet_->four_way_valve_status = 0;
    this->big_data_packet_->indoor_electric_heating_status = 0;
    this->big_data_packet_->expansion_valve_open_degree[0] = 0;
    this->big_data_packet_->expansion_valve_open_degree[1] = 0;
    this->last_alarm_message_ = std::chrono::steady_clock::now();
    this->alarm_paused_ = false;
    this->alarm_stopped_ = true;
}


bool HonServer::has_active_alarms() const{
  for (int i = 0; i < ALARM_BUF_SIZE; i++)
    if (this->alarm_status_buf_[i] != 0)
      return true;
  return false;
}

bool HonServer::is_in_configuration_mode() const {
  return this->config_mode_;
}

void HonServer::process_alarms_()
{
  if (!this->alarm_stopped_ && !this->protocol_handler_->is_waiting_for_answer() && (this->protocol_handler_->get_outgoing_queue_size() == 0)) {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (( this->alarm_paused_ && (std::chrono::duration_cast<std::chrono::milliseconds>(now - this->last_alarm_message_).count() > LONG_ALARM_REPORT_INTERVAL_MS)) ||
        (!this->alarm_paused_ && (std::chrono::duration_cast<std::chrono::milliseconds>(now - this->last_alarm_message_).count() > SHORT_ALARM_REPORT_INTERVAL_MS))) {
      this->alarm_paused_ = false;
      this->protocol_handler_->send_message(haier_protocol::HaierMessage(haier_protocol::FrameType::ALARM_STATUS, 0x0F5A, this->alarm_status_buf_, ALARM_BUF_SIZE), true);
      this->last_alarm_message_ = now;
    }
  }
}

bool HonServer::start_alarm(uint8_t alarm_id) {
  if (alarm_id >= esphome::haier::hon_protocol::HON_ALARM_COUNT)
    return false;
  this->alarm_paused_ = false;
  this->alarm_stopped_ = false;
  this->alarm_status_buf_[ALARM_BUF_SIZE - 1 - (uint8_t)(alarm_id / 8)] |= (1 << (alarm_id % 8));
  return true;
}

void HonServer::reset_alarms() {
  memset(this->alarm_status_buf_, 0, ALARM_BUF_SIZE);
  this->alarm_paused_ = false;
}

const uint8_t* HonServer::get_status_message_buffer() const {
	return this->status_message_buffer_;
}

HvacState HonServer::get_ac_state() const {
	HvacState state;
	state.control = *this->control_packet_;
	state.sensors = *this->sensors_packet_;
	state.big_data = *this->big_data_packet_;
	return state;
}

void HonServer::loop() {
	this->protocol_handler_->loop();
	this->process_alarms_();
}

haier_protocol::HandlerError HonServer::get_device_version_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
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
      this->protocol_handler_->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::GET_DEVICE_VERSION_RESPONSE, device_version_info_buf, sizeof(device_version_info_buf)), true);
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
        this->protocol_handler_->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  } else {
      this->protocol_handler_->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError HonServer::get_device_id_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::GET_DEVICE_ID) {
    if (size == 0) {
      static const uint8_t device_id_buf[] = { 0x20, 0x20, 0x62, 0x84, 0x20, 0xD2, 0x85, 0x34, 0x02, 0x12, 0x71, 0xFB, 0xE0, 0xF4, 0x0D, 0x00,
                                  0x00, 0x00, 0x82, 0x0C, 0xC8, 0x1B, 0xF1, 0x3C, 0x46, 0xAB, 0x92, 0x5B, 0xCE, 0x95, 0x77, 0xC0, // TypeID, 32 bytes binary value (automatically generated when a device is created)
                                  0x04 // Device role (accessory device)
      };
      this->protocol_handler_->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::GET_DEVICE_ID_RESPONSE, device_id_buf, sizeof(device_id_buf)), true);
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
      this->protocol_handler_->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    }
  } else {
    this->protocol_handler_->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError HonServer::status_request_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::CONTROL) {
    if (size < 2) {
        this->protocol_handler_->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
    uint16_t subcommand = (buffer[0] << 8) | buffer[1];
    switch (subcommand) {
    case (uint16_t)SubcommandsControl::GET_USER_DATA:
      if (size != 2) {
        this->protocol_handler_->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      this->protocol_handler_->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D01, this->status_message_buffer_, this->protocol_settings_.status_message_header_size + this->protocol_settings_.control_packet_size + this->protocol_settings_.sensors_packet_size));
      return haier_protocol::HandlerError::HANDLER_OK;
    case (uint16_t)SubcommandsControl::GET_BIG_DATA:
      if (size != 2) {
          this->protocol_handler_->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      this->protocol_handler_->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x7D01, this->status_message_buffer_, this->protocol_settings_.status_message_header_size + this->protocol_settings_.control_packet_size + this->protocol_settings_.sensors_packet_size + this->protocol_settings_.big_data_packet_size));
      return haier_protocol::HandlerError::HANDLER_OK;
    case (uint16_t)SubcommandsControl::SET_GROUP_PARAMETERS:
      if (size - 2 != sizeof(HaierPacketControl)) {
        HAIER_LOGW("Wrong control packet size, expected %d, received %d", sizeof(HaierPacketControl), size - 2);
        this->protocol_handler_->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
      }
      for (unsigned int i = 0; i < sizeof(HaierPacketControl); i++) {
        uint8_t& cbyte = ((uint8_t*)this->control_packet_)[i];
        if (cbyte != buffer[2 + i]) {
          HAIER_LOGI("Byte #%d changed 0x%02X => 0x%02X", i + 10, cbyte, buffer[2 + i]);
          cbyte = buffer[2 + i];
        }
      }
      this->protocol_handler_->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D5F, this->status_message_buffer_, this->protocol_settings_.status_message_header_size + this->protocol_settings_.control_packet_size + this->protocol_settings_.sensors_packet_size));
      return haier_protocol::HandlerError::HANDLER_OK;
    default:
      if ((subcommand & 0xFF00) == (uint16_t)SubcommandsControl::SET_SINGLE_PARAMETER) {
        if (size != 4) {
          HAIER_LOGW("Wrong control packet size, expected 2, received %d", size - 2);
          this->protocol_handler_->send_answer(INVALID_MSG);
          return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
        }
        uint8_t parameter = buffer[1];
        uint16_t value = (buffer[2] << 8) + buffer[3];
        return this->process_single_parameter_(parameter, value);
      }
      else {
          this->protocol_handler_->send_answer(INVALID_MSG);
        return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
      }
    }
  } else {
      this->protocol_handler_->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError HonServer::alarm_status_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::GET_ALARM_STATUS) {
    if (size == 0) {
        this->protocol_handler_->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::GET_ALARM_STATUS_RESPONSE, 0x0F5A, this->alarm_status_buf_, ALARM_BUF_SIZE));
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
        this->protocol_handler_->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  } else {
      this->protocol_handler_->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError HonServer::get_management_information_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION) {
    if (size == 0) {
      static const uint8_t management_information_buf[] = { 
        0x00, // Mode switch (normal working condition)
        0x00, // Reserved
        0x00, // Clear user information (no action)
        0x00, // Forced setting (not mandatory)
        0x00, 0x00 // Reserved
      };
      this->protocol_handler_->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION_RESPONSE, management_information_buf, sizeof(management_information_buf)));
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
        this->protocol_handler_->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    }
  } else {
      this->protocol_handler_->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError HonServer::report_network_status_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::REPORT_NETWORK_STATUS) {
    if (size == 4) {
      uint8_t st = buffer[1];
      this->config_mode_ = st == 3;
      if (!this->config_mode_ && (this->control_packet_->set_point == 0x0E))
        this->control_packet_->set_point = 0x0D;
      if (st != this->communication_status_) {
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
        this->communication_status_ = st;
      }
      this->protocol_handler_->send_answer(CONFIRM_MSG);
      return haier_protocol::HandlerError::HANDLER_OK;
    } else {
        this->protocol_handler_->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  } else {
      this->protocol_handler_->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError HonServer::alarm_status_report_answer_handler_(haier_protocol::FrameType request_type, haier_protocol::FrameType message_type, const uint8_t* data, size_t data_size) {
  if (request_type == haier_protocol::FrameType::ALARM_STATUS) {
    if (message_type == haier_protocol::FrameType::CONFIRM) {
      if (data_size == 0) {
        if (!has_active_alarms())
          this->alarm_stopped_ = true;
        else
          this->alarm_paused_ = true;
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

haier_protocol::HandlerError HonServer::stop_alarm_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size) {
  if (type == haier_protocol::FrameType::STOP_FAULT_ALARM) {
    if (size == 0) {
      HAIER_LOGI("Stop alarm message received.");
      this->alarm_stopped_ = true;
      this->protocol_handler_->send_answer(CONFIRM_MSG);
      return haier_protocol::HandlerError::HANDLER_OK;
    }
    else {
        this->protocol_handler_->send_answer(INVALID_MSG);
      return haier_protocol::HandlerError::WRONG_MESSAGE_STRUCTURE;
    }
  }
  else {
      this->protocol_handler_->send_answer(INVALID_MSG);
    return haier_protocol::HandlerError::UNSUPPORTED_MESSAGE;
  }
}

haier_protocol::HandlerError HonServer::process_single_parameter_(uint8_t parameter, uint16_t value)
{
  #define SET_IF_DIFFERENT(VALUE, FIELD) \
      do { \
        if (this->control_packet_->FIELD != VALUE) { \
          HAIER_LOGI(#FIELD" <= %u", VALUE); \
          this->control_packet_->FIELD = VALUE; \
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
      this->protocol_handler_->send_answer(haier_protocol::HaierMessage(haier_protocol::FrameType::STATUS, 0x6D01, this->status_message_buffer_, this->protocol_settings_.status_message_header_size + this->protocol_settings_.control_packet_size + this->protocol_settings_.sensors_packet_size));
  }
  else {
      this->protocol_handler_->send_answer(INVALID_MSG);
  }
  return result;
  #undef SET_IF_DIFFERENT
}
