#pragma once

#include <stdint.h>
#include "protocol/haier_protocol.h"
#include "hon_packet.h"
#include "server_base.h"

struct HvacState {
  esphome::haier::hon_protocol::HaierPacketControl control;
  esphome::haier::hon_protocol::HaierPacketSensors sensors;
  esphome::haier::hon_protocol::HaierPacketBigData big_data;
};

constexpr size_t ALARM_BUF_SIZE = 8;

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
  uint8_t get_total_status_data_size() const { return status_message_header_size + control_packet_size + sensors_packet_size + big_data_packet_size; };
};

class HonServer: public HaierBaseServer{
public:
  HonServer() = delete;
  HonServer(haier_protocol::ProtocolStream&);
  HonServer(haier_protocol::ProtocolStream&, HonProtocolSettings);
  bool is_in_configuration_mode() const;
  bool start_alarm(uint8_t alarm_id);
  void reset_alarms();
  bool has_active_alarms() const;
  const uint8_t* get_status_message_buffer() const;
  HvacState get_hvac_state() const;
  const HonProtocolSettings& get_protocol_settings() const { return this->protocol_settings_; };
  void set_hvac_state(const HvacState& state);
  void loop() override;
protected:
  virtual void register_handlers_();
  void init_ac_state_internal_();
  void process_alarms_();
  // Handlers
  haier_protocol::HandlerError get_device_version_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size);
  haier_protocol::HandlerError get_device_id_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size);
  haier_protocol::HandlerError status_request_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size);
  haier_protocol::HandlerError alarm_status_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size);
  haier_protocol::HandlerError get_management_information_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size);
  haier_protocol::HandlerError report_network_status_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size);
  haier_protocol::HandlerError stop_alarm_handler_(haier_protocol::FrameType type, const uint8_t* buffer, size_t size);
  haier_protocol::HandlerError process_single_parameter_(uint8_t parameter, uint16_t value);
  haier_protocol::HandlerError alarm_status_report_answer_handler_(haier_protocol::FrameType request_type, haier_protocol::FrameType message_type, const uint8_t* data, size_t data_size);
private:
  HonProtocolSettings protocol_settings_;
  uint8_t* status_message_buffer_;
  uint8_t status_message_buffer_size_;
  esphome::haier::hon_protocol::HaierPacketControl* control_packet_;
  esphome::haier::hon_protocol::HaierPacketSensors* sensors_packet_;
  esphome::haier::hon_protocol::HaierPacketBigData* big_data_packet_;
  uint8_t communication_status_{ 0xFF };
  std::chrono::steady_clock::time_point last_alarm_message_;
  bool alarm_paused_;
  bool alarm_stopped_;
  bool config_mode_{ false };
  uint8_t alarm_status_buf_[ALARM_BUF_SIZE];
};

