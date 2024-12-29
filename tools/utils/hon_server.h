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

struct HvacState {
  esphome::haier::hon_protocol::HaierPacketControl control;
  esphome::haier::hon_protocol::HaierPacketSensors sensors;
  esphome::haier::hon_protocol::HaierPacketBigData big_data;
};

constexpr size_t ALARM_BUF_SIZE = 8;

constexpr size_t USER_DATA_SIZE = (CONF_CONTROL_PACKET_SIZE + CONF_SENSORS_PACKET_SIZE);

constexpr size_t BIG_DATA_SIZE = CONF_BIG_DATA_PACKET_SIZE;

constexpr size_t TOTAL_PACKET_SIZE = CONF_STATUS_MESSAGE_HEADER_SIZE + CONF_CONTROL_PACKET_SIZE + CONF_SENSORS_PACKET_SIZE + CONF_BIG_DATA_PACKET_SIZE;

struct HonProtocolSettings {
  uint8_t status_message_header_size;
  uint8_t control_packet_size;
  uint8_t sensors_packet_size;
  uint8_t big_data_packet_size;
  bool encription;    // Not supported yet
  bool crc;
  HonProtocolSettings() :
    status_message_header_size(0),
    control_packet_size(sizeof(esphome::haier::hon_protocol::HaierPacketControl)),
    sensors_packet_size(sizeof(esphome::haier::hon_protocol::HaierPacketSensors) + 4),
    big_data_packet_size(sizeof(esphome::haier::hon_protocol::HaierPacketBigData)),
    encription(false),
    crc(true)
  {}
  uint8_t get_total_status_data_size() { return status_message_header_size + control_packet_size + sensors_packet_size + big_data_packet_size; };
};

class HonServer {
public:
  HonServer() = delete;
  HonServer(haier_protocol::ProtocolStream&);
  HonServer(haier_protocol::ProtocolStream&, HonProtocolSettings);
  ~HonServer();
  bool is_in_configuration_mode() const;
  bool start_alarm(uint8_t alarm_id);
  void reset_alarms();
  const uint8_t* get_status_message_buffer() const;
  HvacState gety_ac_state() const;
  const HonProtocolSettings& get_protocol_settings() const;
private:
  HonProtocolSettings protocol_settings_;
  haier_protocol::ProtocolHandler* protocol_handler_;
  uint8_t* status_message_buffer_;
};

void process_alarms(haier_protocol::ProtocolHandler* protocol_handler);

HvacFullStatus init_ac_state(uint8_t* buffer, size_t buffer_size);

bool start_alarm(uint8_t alarm_id);

void reset_alarms();

bool is_in_configuration_mode();

haier_protocol::HandlerError get_device_version_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError get_device_id_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler* protocol_handler, HvacFullStatus& ac_state, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError alarm_status_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError get_management_information_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError report_network_status_handler(haier_protocol::ProtocolHandler* protocol_handler, HvacFullStatus& ac_state, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError stop_alarm_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError process_single_parameter(haier_protocol::ProtocolHandler* protocol_handler, HvacFullStatus& ac_state, uint8_t parameter, uint16_t value);

haier_protocol::HandlerError alarm_status_report_answer_handler(haier_protocol::ProtocolHandler* protocol_handler, haier_protocol::FrameType request_type, haier_protocol::FrameType message_type, const uint8_t* data, size_t data_size);