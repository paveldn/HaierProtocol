#include <stdint.h>
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include "virtual_stream.h"
#include "protocol/haier_protocol.h"
#include "smartair2_packet.h"
#include "smartair2_server.h"
#include "console_log.h"

using namespace esphome::haier::smartair2_protocol;

HaierPacketControl ac_full_state;

uint8_t expected_answers[][2] = {
	{(uint8_t)FrameType::GET_DEVICE_VERSION, (uint8_t)FrameType::GET_DEVICE_VERSION_RESPONSE},
	{(uint8_t)FrameType::CONTROL, (uint8_t)FrameType::STATUS},
	{(uint8_t)FrameType::REPORT_NETWORK_STATUS, (uint8_t)FrameType::CONFIRM},
	{0, 0}
};

haier_protocol::HandlerError client_answers_handler(uint8_t message_type, uint8_t answer_type, const uint8_t* buffer, size_t size) {
	unsigned int ind = 0;
	while (expected_answers[ind][0] != 0) {
		if (message_type == expected_answers[ind][0])
			break;
		ind++;
	};
	if (expected_answers[ind][0] == 0) {
		HAIER_LOGW("Unexpected command 0x%02X", message_type);
		return haier_protocol::HandlerError::UNEXPECTED_MESSAGE;
	}
	if (expected_answers[ind][1] != answer_type) {
		HAIER_LOGW("Unexpected answer 0x%02X for command 0x%02X", answer_type, message_type);
		return haier_protocol::HandlerError::INVALID_ANSWER;
	}
	HAIER_LOGI("Answer 0x%02X received for command 0x%02X", answer_type, message_type);
	return haier_protocol::HandlerError::HANDLER_OK;
}

int main(int argc, char** argv) {
	VirtualStreamHolder stream_holder;
	haier_protocol::set_log_handler(console_logger);
	VirtualStream& server_stream = stream_holder.get_stream_reference(StreamDirection::DIRECTION_A);
	VirtualStream& client_stream = stream_holder.get_stream_reference(StreamDirection::DIRECTION_B);
	haier_protocol::ProtocolHandler smartair2_server(server_stream);
	smartair2_server.set_message_handler((uint8_t)FrameType::GET_DEVICE_VERSION, std::bind(get_device_version_handler, &smartair2_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	smartair2_server.set_message_handler((uint8_t)FrameType::CONTROL, std::bind(status_request_handler, &smartair2_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	smartair2_server.set_message_handler((uint8_t)FrameType::REPORT_NETWORK_STATUS, std::bind(report_network_status_handler, &smartair2_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	haier_protocol::ProtocolHandler smartair2_client(client_stream);
	ac_full_state = get_ac_state_ref();
	smartair2_client.set_default_answer_handler(client_answers_handler);
	{
		uint8_t module_capabilities[2] = { 0b00000000, 0b00000111 };
		const haier_protocol::HaierMessage device_version_request_message((uint8_t)FrameType::GET_DEVICE_VERSION, module_capabilities, sizeof(module_capabilities));
		smartair2_client.send_message(device_version_request_message, false, 100);
		smartair2_client.loop();
		smartair2_server.loop();
		smartair2_client.loop();
		smartair2_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		const haier_protocol::HaierMessage status_request_message((uint8_t)FrameType::CONTROL, 0x4D01);
		smartair2_client.send_message(status_request_message, false);
		smartair2_client.loop();
		smartair2_server.loop();
		smartair2_client.loop();
		smartair2_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		const uint8_t wifi_status_data[4] = { 0x00, 0x01, 0x00, 0x37 };
		haier_protocol::HaierMessage wifi_status_request((uint8_t)FrameType::REPORT_NETWORK_STATUS, wifi_status_data, sizeof(wifi_status_data));
		smartair2_client.send_message(wifi_status_request, false);
		smartair2_client.loop();
		smartair2_server.loop();
		smartair2_client.loop();
		smartair2_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		ac_full_state.ac_power = true;
		ac_full_state.ac_mode = (uint8_t)ConditioningMode::COOL;
		ac_full_state.display_status = 0;
		haier_protocol::HaierMessage control_message((uint8_t)FrameType::CONTROL, 0x4D5F, (uint8_t*)&ac_full_state, sizeof(HaierPacketControl));
		smartair2_client.send_message(control_message, false);
		smartair2_client.loop();
		smartair2_server.loop();
		smartair2_client.loop();
		smartair2_server.loop();
	}
	if (memcmp(&ac_full_state, &get_ac_state_ref(), sizeof(HaierPacketControl)) == 0) {
		HAIER_LOGI("AC control processed correctly");
	}
	else {
		HAIER_LOGW("AC control not OK");
	}
	unsigned int warn = get_warnings_count();
	unsigned int  errors = get_errors_count();
	std::cout << "Test results, warning: " << warn << " errors: " << errors << std::endl;
	if ((warn != 2) || (errors != 0))
		exit(1);
}

