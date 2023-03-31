﻿#pragma once

#include <stdint.h>
#include "protocol/haier_protocol.h"
#include "smartair2_packet.h"

esphome::haier::smartair2_protocol::HaierPacketControl& get_ac_state_ref();

haier_protocol::HandlerError get_device_version_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size);

haier_protocol::HandlerError report_network_status_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size);