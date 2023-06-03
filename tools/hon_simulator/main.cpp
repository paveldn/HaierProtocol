#include <stdint.h>
#include "simulator_base.h"
#include "hon_packet.h"
#include "hon_server.h"
#include <iostream>
#include <string>

using namespace esphome::haier::hon_protocol;
HvacFullStatus& ac_state = get_ac_state_ref();


bool _enter_config_mode{ false };
bool _toggle_ac_power{ false };
bool _trigger_random_alarm{ false };

void preloop(haier_protocol::ProtocolHandler* handler) {
  if (_toggle_ac_power) {
    _toggle_ac_power = false;
    if (!is_in_configuration_mode()) {
      uint8_t ac_power = ac_state.control.ac_power;
      ac_state.control.ac_power = ac_power == 1 ? 0 : 1;
      HAIER_LOGI("AC power is %s", ac_power == 1 ? "Off" : "On");
    } else {
      HAIER_LOGW("Can't change AC power when in configuration mode!");
    }
  }
  if (_enter_config_mode) {
    _enter_config_mode = false;
    if (!is_in_configuration_mode()) {
      HAIER_LOGI("Entering pairing mode");
      ac_state.control.set_point = 0x0E;
      ac_state.control.vertical_swing_mode = (uint8_t) VerticalSwingMode::MAX_UP;
      ac_state.control.fan_mode = (uint8_t)FanMode::FAN_LOW;
      ac_state.control.special_mode = (uint8_t)SpecialMode::NONE;
      ac_state.control.ac_mode = (uint8_t)ConditioningMode::COOL;
      ac_state.control.ten_degree = 0;
      ac_state.control.display_status = 1;
      ac_state.control.half_degree = 0;
      ac_state.control.intelligence_status = 0;
      ac_state.control.pmv_status = 0;
      ac_state.control.use_fahrenheit = 0;
      ac_state.control.ac_power = 1;
      ac_state.control.health_mode = 1;
      ac_state.control.electric_heating_status = 0;
      ac_state.control.fast_mode = 0;
      ac_state.control.quiet_mode = 0;
      ac_state.control.sleep_mode = 0;
      ac_state.control.lock_remote = 0;
      ac_state.control.beeper_status = 0;
      ac_state.control.horizontal_swing_mode = (uint8_t)HorizontalSwingMode::CENTER;
      ac_state.control.fresh_air_status = 0;
      ac_state.control.humidification_status = 0;
      ac_state.control.pm2p5_cleaning_status = 1;
      ac_state.control.ch2o_cleaning_status = 1;
      ac_state.control.self_cleaning_status = 0;
      ac_state.control.light_status = 0;
      ac_state.control.energy_saving_status = 0;
      ac_state.control.cleaning_time_status = 0;
    }
  }
  if (_trigger_random_alarm) {
    _trigger_random_alarm = false;
    HAIER_LOGI("Random alarm triggered");
    trigger_random_alarm();
  }
}

void main(int argc, char** argv) {
  if (argc == 2) {
    std::srand(std::time(nullptr));
    message_handlers mhandlers;
    mhandlers[(uint8_t)FrameType::GET_DEVICE_VERSION] = get_device_version_handler;
    mhandlers[(uint8_t)FrameType::GET_DEVICE_ID] = get_device_id_handler;
    mhandlers[(uint8_t)FrameType::CONTROL] = status_request_handler;
    mhandlers[(uint8_t)FrameType::GET_ALARM_STATUS] = alarm_status_handler;
    mhandlers[(uint8_t)FrameType::GET_MANAGEMENT_INFORMATION] = get_management_information_handler;
    mhandlers[(uint8_t)FrameType::REPORT_NETWORK_STATUS] = report_network_status_handler;
    keyboard_handlers khandlers;
    khandlers['1'] = []() { _toggle_ac_power = true; };
    khandlers['2'] = []() { _enter_config_mode = true; };
    khandlers['3'] = []() {
      ac_state.control.self_cleaning_status = false;
      ac_state.control.steri_clean = false; 
    };
    khandlers['a'] = []() { _trigger_random_alarm = true; };
    simulator_main("hOn HVAC simulator", argv[1], mhandlers, khandlers, preloop);
  }
  else {
    std::cout << "Please use: hon_simulator <port>" << std::endl;
  }
}
