#include <stdint.h>
#include "simulator_base.h"
#include "hon_packet.h"
#include "hon_server.h"
#include <iostream>
#include <string>

using namespace esphome::haier::hon_protocol;
HvacFullStatus& ac_state = get_ac_state_ref();

enum class PiringMode {
  NONE = 0,
  HON_PAIRING = 1,
};

PiringMode _pairing_mode{ PiringMode::NONE };
bool _toggle_ac_power{ false };
bool _trigger_random_alarm{ false };
bool _reset_alarm{ false };

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
  if (_pairing_mode == PiringMode::HON_PAIRING) {
    _pairing_mode = PiringMode::NONE;
    if (!is_in_configuration_mode()) {
      HAIER_LOGI("Entering hOn pairing mode");
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
    size_t r = esphome::haier::hon_protocol::HON_ALARM_COUNT -   std::rand() % (esphome::haier::hon_protocol::HON_ALARM_COUNT);
    HAIER_LOGI("Random alarm triggered. Alarm code %d", r);
    start_alarm(r);
  }
  if (_reset_alarm) {
    _reset_alarm = false;
    HAIER_LOGI("Reseting all alarms");
    reset_alarms();
  }
  process_alarms(handler);
}

int main(int argc, char** argv) {
  if (argc == 2) {
    std::srand(std::time(nullptr));
    message_handlers mhandlers;
    mhandlers[haier_protocol::FrameType::GET_DEVICE_VERSION] = get_device_version_handler;
    mhandlers[haier_protocol::FrameType::GET_DEVICE_ID] = get_device_id_handler;
    mhandlers[haier_protocol::FrameType::CONTROL] = status_request_handler;
    mhandlers[haier_protocol::FrameType::GET_ALARM_STATUS] = alarm_status_handler;
    mhandlers[haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION] = get_management_information_handler;
    mhandlers[haier_protocol::FrameType::REPORT_NETWORK_STATUS] = report_network_status_handler;
    mhandlers[haier_protocol::FrameType::STOP_FAULT_ALARM] = stop_alarm_handler;
    answer_handlers ahandlers;
    ahandlers[haier_protocol::FrameType::ALARM_STATUS] = alarm_status_report_answer_handler;
    keyboard_handlers khandlers;
    khandlers['1'] = []() { _toggle_ac_power = true; };
    khandlers['2'] = []() { _pairing_mode = PiringMode::HON_PAIRING; };
    khandlers['3'] = []() {
      ac_state.control.self_cleaning_status = false;
      ac_state.control.steri_clean = false; 
    };
    khandlers['4'] = []() {
      ac_state.control.quiet_mode = 1 - ac_state.control.quiet_mode;
      HAIER_LOGI("Quiet mode is %s", ac_state.control.quiet_mode  == 1 ? "On" : "Off");
    };
    khandlers['5'] = []() {
      ac_state.control.health_mode = 1 - ac_state.control.health_mode;
      HAIER_LOGI("Health mode is %s", ac_state.control.health_mode == 1 ? "On" : "Off");
    };
    khandlers['a'] = []() { _trigger_random_alarm = true; };
    khandlers['s'] = []() { _reset_alarm = true; };
    simulator_main("hOn HVAC simulator", argv[1], mhandlers, ahandlers, khandlers, preloop);
  }
  else {
    std::cout << "Please use: hon_simulator <port>" << std::endl;
  }
}
