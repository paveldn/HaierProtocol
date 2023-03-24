#include <stdint.h>
#include "simulator_base.h"
#include "hon_packet.h"
#include "hon_server.h"
#include <iostream>
#include <string>

using namespace esphome::haier::hon_protocol;
HvacFullStatus& ac_state = get_ac_state_ref();

bool _toggle_pairing_mode{ false };
bool _toggle_ac_power{ false };
bool _trigger_random_alarm{ false };

void preloop(haier_protocol::ProtocolHandler* handler) {
  static bool is_in_pairing_mode = false;
  if (_toggle_ac_power) {
    _toggle_ac_power = false;
    uint8_t ac_power = ac_state.control.ac_power;
    ac_state.control.ac_power = ac_power == 1 ? 0 : 1;
    HAIER_LOGI("AC power is %s", ac_power == 1 ? "Off" : "On");
  }
  if (_toggle_pairing_mode) {
    _toggle_pairing_mode = false;
    if (is_in_pairing_mode) {
      HAIER_LOGI("Entering working mode");
      ac_state.control.ac_power = 0;
      ac_state.control.vertical_swing_mode = (uint8_t)VerticalSwingMode::AUTO;
      ac_state.control.fan_mode = (uint8_t)FanMode::FAN_LOW;
      ac_state.control.set_point = 25 - 16;
      uint8_t cmd_buf[] = { 0x00, 0x00 };
      static const haier_protocol::HaierMessage WORKING_MODE_MSG((uint8_t)FrameType::STOP_WIFI_CONFIGURATION, cmd_buf, 2);
      handler->send_message(WORKING_MODE_MSG, true);
    }
    else {
      HAIER_LOGI("Entering pairing mode");
      ac_state.control.ac_power = 1;
      ac_state.control.ac_mode = (uint8_t)ConditioningMode::COOL;
      ac_state.control.set_point = 0x0E;
      ac_state.control.vertical_swing_mode = 0x0A;
      ac_state.control.fan_mode = (uint8_t)FanMode::FAN_LOW;
      uint8_t cmd_buf[] = {0x00, 0x00 };
      static const haier_protocol::HaierMessage CONFIGURATION_MODE_MSG((uint8_t)FrameType::START_WIFI_CONFIGURATION, cmd_buf, 2);
      handler->send_message(CONFIGURATION_MODE_MSG, true);
    }
    is_in_pairing_mode = !is_in_pairing_mode;
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
    mhandlers[(uint8_t)FrameType::GET_MANAGEMENT_INFORMATION] = get_managment_information_handler;
    mhandlers[(uint8_t)FrameType::REPORT_NETWORK_STATUS] = report_network_status_handler;
    keyboard_handlers khandlers;
    khandlers['1'] = []() { _toggle_ac_power = true; };
    khandlers['2'] = []() { _toggle_pairing_mode = true; };
    khandlers['a'] = []() { _trigger_random_alarm = true; };
    simulator_main("hOn HVAC simulator", argv[1], mhandlers, khandlers, preloop);
  }
  else {
    std::cout << "Please use: hon_simulator <port>" << std::endl;
  }
}
