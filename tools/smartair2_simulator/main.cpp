#include <stdint.h>
#include "simulator_base.h"
#include "smartair2_packet.h"
#include "smartair2_server.h"
#include <string>
#include <iostream>

using namespace esphome::haier::smartair2_protocol;

HaierPacketControl& ac_state = get_ac_state_ref();

const haier_protocol::HaierMessage INVALID_MSG(haier_protocol::FrameType::INVALID, 0x0000);
const haier_protocol::HaierMessage CONFIRM_MSG(haier_protocol::FrameType::CONFIRM);
bool toggle_ac_power{ false };
bool start_pairing{ false };

void preloop(haier_protocol::ProtocolHandler* handler) {
  if (toggle_ac_power) {
    toggle_ac_power = false;
    uint8_t ac_power = ac_state.ac_power;
    ac_state.ac_power = ac_power == 1 ? 0 : 1;
    HAIER_LOGI("AC power is %s", ac_power == 1 ? "Off" : "On");
  }
  if (start_pairing) {
    start_pairing = false;
    ac_state.ac_power = 1;
    ac_state.set_point = 14;
    ac_state.ac_mode = (uint8_t) esphome::haier::smartair2_protocol::ConditioningMode::COOL;
    ac_state.fan_mode = (uint8_t) esphome::haier::smartair2_protocol::FanMode::FAN_LOW;
    HAIER_LOGI("Start pairing");
  }
}

int main(int argc, char** argv) {
  if (argc == 2) {
    message_handlers mhandlers;
    mhandlers[haier_protocol::FrameType::GET_DEVICE_VERSION] = unsupported_message_handler;
    mhandlers[haier_protocol::FrameType::GET_DEVICE_ID] = unsupported_message_handler;
    mhandlers[haier_protocol::FrameType::CONTROL] = status_request_handler;
    mhandlers[haier_protocol::FrameType::REPORT_NETWORK_STATUS] = report_network_status_handler;

    keyboard_handlers khandlers;
    khandlers['1'] = []() { toggle_ac_power = true; };
    khandlers['2'] = []() { start_pairing = true; };
    simulator_main("SmartAir2 HVAC simulator", argv[1], mhandlers, khandlers, preloop);
  } else {
    std::cout << "Please use: smartair2_simulator <port>" << std::endl;
  }
}
