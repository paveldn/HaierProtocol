#pragma once

#include <functional>
#include <unordered_map>
#include "protocol/haier_protocol.h"

using message_handlers = std::unordered_map<haier_protocol::FrameType, std::function<haier_protocol::HandlerError(haier_protocol::ProtocolHandler*, haier_protocol::FrameType, const uint8_t*, size_t)>>;
using answer_handlers = std::unordered_map<haier_protocol::FrameType, std::function<haier_protocol::HandlerError(haier_protocol::ProtocolHandler*, haier_protocol::FrameType, haier_protocol::FrameType, const uint8_t*, size_t)>>;
using keyboard_handlers = std::unordered_map<char, std::function<void()>>;
using protocol_preloop = std::function<void(haier_protocol::ProtocolHandler*)>;

void simulator_main(const char* app_name, const char* port_name, message_handlers mhandlers, keyboard_handlers khandlers, protocol_preloop ploop);
void simulator_main(const char* app_name, const char* port_name, message_handlers mhandlers, answer_handlers ahandlers, keyboard_handlers khandlers, protocol_preloop ploop);
