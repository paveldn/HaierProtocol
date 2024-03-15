#include <stdint.h>
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include "virtual_stream.h"
#include "protocol/haier_protocol.h"
#include "hon_packet.h"
#include "hon_server.h"
#include "console_log.h"

using namespace esphome::haier::hon_protocol;

HvacFullStatus ac_full_state;

haier_protocol::FrameType expected_answers[][2] = {
	{haier_protocol::FrameType::GET_DEVICE_VERSION, haier_protocol::FrameType::GET_DEVICE_VERSION_RESPONSE},
	{haier_protocol::FrameType::GET_DEVICE_ID, haier_protocol::FrameType::GET_DEVICE_ID_RESPONSE},
	{haier_protocol::FrameType::CONTROL, haier_protocol::FrameType::STATUS},
	{haier_protocol::FrameType::GET_ALARM_STATUS, haier_protocol::FrameType::GET_ALARM_STATUS_RESPONSE},
	{haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION, haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION_RESPONSE},
	{haier_protocol::FrameType::REPORT_NETWORK_STATUS, haier_protocol::FrameType::CONFIRM},
	{haier_protocol::FrameType::UNKNOWN_FRAME_TYPE, haier_protocol::FrameType::UNKNOWN_FRAME_TYPE}
};

haier_protocol::HandlerError client_answers_handler(haier_protocol::FrameType message_type, haier_protocol::FrameType answer_type, const uint8_t* buffer, size_t size) {
	unsigned int ind = 0;
	while (expected_answers[ind][0] != haier_protocol::FrameType::UNKNOWN_FRAME_TYPE) {
		if (message_type == expected_answers[ind][0])
			break;
		ind++;
	};
	if (expected_answers[ind][0] == haier_protocol::FrameType::UNKNOWN_FRAME_TYPE) {
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
	haier_protocol::ProtocolHandler hon_server(server_stream);
	hon_server.set_message_handler(haier_protocol::FrameType::GET_DEVICE_VERSION, std::bind(get_device_version_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler(haier_protocol::FrameType::GET_DEVICE_ID, std::bind(get_device_id_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler(haier_protocol::FrameType::CONTROL, std::bind(status_request_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler(haier_protocol::FrameType::GET_ALARM_STATUS, std::bind(alarm_status_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler(haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION, std::bind(get_management_information_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler(haier_protocol::FrameType::REPORT_NETWORK_STATUS, std::bind(report_network_status_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	haier_protocol::ProtocolHandler hon_client(client_stream);
	ac_full_state = get_ac_state_ref();
	hon_client.set_default_answer_handler(client_answers_handler);
	{
		uint8_t module_capabilities[2] = { 0b00000000, 0b00000111 };
		const haier_protocol::HaierMessage device_version_request_message(haier_protocol::FrameType::GET_DEVICE_VERSION, module_capabilities, sizeof(module_capabilities));
		hon_client.send_message(device_version_request_message, false);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		const haier_protocol::HaierMessage device_request_message(haier_protocol::FrameType::GET_DEVICE_ID);
		hon_client.send_message(device_request_message, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		const haier_protocol::HaierMessage status_request_message(haier_protocol::FrameType::CONTROL, (uint16_t)SubcommandsControl::GET_USER_DATA);
		hon_client.send_message(status_request_message, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		const haier_protocol::HaierMessage alarm_status_request_message(haier_protocol::FrameType::GET_ALARM_STATUS);
		hon_client.send_message(alarm_status_request_message, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		const haier_protocol::HaierMessage update_signal_request(haier_protocol::FrameType::GET_MANAGEMENT_INFORMATION);
		hon_client.send_message(update_signal_request, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		const uint8_t wifi_status_data[4] = { 0x00, 0x01, 0x00, 0x37 };
		haier_protocol::HaierMessage wifi_status_request(haier_protocol::FrameType::REPORT_NETWORK_STATUS, wifi_status_data, sizeof(wifi_status_data));
		hon_client.send_message(wifi_status_request, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		ac_full_state.control.ac_power = true;
		ac_full_state.control.ac_mode = (uint8_t) ConditioningMode::HEALTHY_DRY;
		ac_full_state.control.fan_mode = (uint8_t) FanMode::FAN_MID;
		ac_full_state.control.horizontal_swing_mode = (uint8_t)HorizontalSwingMode::MAX_LEFT;
		ac_full_state.control.vertical_swing_mode = (uint8_t)VerticalSwingMode::HEALTH_DOWN;
		ac_full_state.control.display_status = 0;
		haier_protocol::HaierMessage control_message(haier_protocol::FrameType::CONTROL, (uint16_t)SubcommandsControl::SET_GROUP_PARAMETERS, (uint8_t*)&ac_full_state.control, sizeof(HaierPacketControl));
		hon_client.send_message(control_message, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	// Some of the appliances (washing machines) take longer than a second to send the status answer
	// For these devices, it is better to request data without waiting for an answerand process the answer as the incoming message.
	// Testing mechanism to request message without expecting answer and process answer as a new message 
	hon_client.set_message_handler(haier_protocol::FrameType::STATUS,
		[] (haier_protocol::FrameType message_type, const uint8_t* data, size_t data_size) -> haier_protocol::HandlerError {
			if (message_type == haier_protocol::FrameType::STATUS)
				HAIER_LOGI("Received an answer as a new message, type 0x%02X", message_type);
			else
				return haier_protocol::default_message_handler(message_type, data, data_size);
			return haier_protocol::HandlerError::HANDLER_OK;
		});
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	{
		const haier_protocol::HaierMessage status_request_message(haier_protocol::FrameType::CONTROL, (uint16_t)SubcommandsControl::GET_USER_DATA);
		hon_client.send_message_without_answer(status_request_message, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}	
	if (memcmp(&ac_full_state.control, &get_ac_state_ref().control, sizeof(HaierPacketControl)) == 0) {
		HAIER_LOGI("AC control processed correctly");
	} else {
		HAIER_LOGW("AC control not OK");
	}
	unsigned int warn = get_warnings_count();
	unsigned int  errors = get_errors_count();
	std::cout << "Test results, warning: " << warn << " errors: " << errors << std::endl;
	if ((warn != 0) || (errors != 0))
		exit(1);
}
