#pragma once

#include <stdint.h>
#include "protocol/haier_protocol.h"
#include "hon_packet.h"


#ifndef CONF_STATUS_MESSAGE_HEADER_SIZE 
#define CONF_STATUS_MESSAGE_HEADER_SIZE 0
#endif

#ifndef CONF_CONTROL_PACKET_SIZE
#define CONF_CONTROL_PACKET_SIZE (sizeof(esphome::haier::hon_protocol::HaierPacketControl))
#endif

#ifndef CONF_SENSORS_PACKET_SIZE 
#define CONF_SENSORS_PACKET_SIZE (sizeof(esphome::haier::hon_protocol::HaierPacketSensors) + 4)
#endif

#ifndef CONF_BIG_DATA_PACKET_SIZE
#define CONF_BIG_DATA_PACKET_SIZE (sizeof(esphome::haier::hon_protocol::HaierPacketBigData))
#endif

struct HvacFullStatus {
  esphome::haier::hon_protocol::HaierPacketControl& control;
  esphome::haier::hon_protocol::HaierPacketSensors& sensors;
  esphome::haier::hon_protocol::HaierPacketBigData& big_data;
};

constexpr size_t ALARM_BUF_SIZE = 8;

constexpr size_t USER_DATA_SIZE = (CONF_CONTROL_PACKET_SIZE + CONF_SENSORS_PACKET_SIZE);

constexpr size_t BIG_DATA_SIZE = CONF_BIG_DATA_PACKET_SIZE;

constexpr size_t TOTAL_PACKET_SIZE = CONF_STATUS_MESSAGE_HEADER_SIZE + CONF_CONTROL_PACKET_SIZE + CONF_SENSORS_PACKET_SIZE + CONF_BIG_DATA_PACKET_SIZE;

void process_alarms(haier_protocol::ProtocolHandler* protocol_handler);

HvacFullStatus& get_ac_state_ref();

bool start_alarm(uint8_t alarm_id);

void reset_alarms();

bool is_in_configuration_mode();

haier_protocol::HandlerError get_device_version_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError get_device_id_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError alarm_status_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError get_management_information_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError report_network_status_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError stop_alarm_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError process_single_parameter(haier_protocol::ProtocolHandler* protocol_handler, uint8_t parameter, uint16_t value);

haier_protocol::HandlerError alarm_status_report_answer_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType request_type, haier_protocol::FrameType message_type, const uint8_t* data, size_t data_size);