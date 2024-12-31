#pragma once

#include <functional>
#include <unordered_map>
#include "base_server.h"

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

using keyboard_handlers = std::unordered_map<char, std::function<void()>>;
using protocol_preloop = std::function<void(HaierBaseServer*)>;

void simulator_main(const char* app_name, const char* port_name, HaierBaseServer* server, keyboard_handlers khandlers, protocol_preloop ploop);
