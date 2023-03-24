#include <stdint.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "virtual_stream.h"
#include "protocol/haier_protocol.h"
#include "hon_packet.h"
#include "hon_server.h"
#include "console_log.h"

using namespace esphome::haier::hon_protocol;

haier_protocol::HandlerError client_answers_handler(uint8_t message_type, uint8_t answer_type, const uint8_t* buffer, size_t size) {
	HAIER_LOGI("Answer 0x%02X received for command 0x%02X", answer_type, message_type);
	return haier_protocol::HandlerError::HANDLER_OK;
}


void main(int argc, char** argv) {
	VirtualStreamHolder stream_holder;
	haier_protocol::set_log_handler(console_logger);
	VirtualStream& server_stream = stream_holder.get_stream_referance(StreamDirection::DIRECTION_A);
	VirtualStream& client_stream = stream_holder.get_stream_referance(StreamDirection::DIRECTION_B);
	haier_protocol::ProtocolHandler hon_server(server_stream);
	hon_server.set_message_handler((uint8_t)FrameType::GET_DEVICE_VERSION, std::bind(get_device_version_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler((uint8_t)FrameType::GET_DEVICE_ID, std::bind(get_device_id_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler((uint8_t)FrameType::CONTROL, std::bind(status_request_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler((uint8_t)FrameType::GET_ALARM_STATUS, std::bind(alarm_status_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler((uint8_t)FrameType::GET_MANAGEMENT_INFORMATION, std::bind(get_managment_information_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hon_server.set_message_handler((uint8_t)FrameType::REPORT_NETWORK_STATUS, std::bind(report_network_status_handler, &hon_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	haier_protocol::ProtocolHandler hon_client(client_stream);
	hon_client.set_default_answer_handler(client_answers_handler);
	{
		uint8_t module_capabilities[2] = { 0b00000000, 0b00000111 };
		const haier_protocol::HaierMessage device_version_request_message((uint8_t)FrameType::GET_DEVICE_VERSION, module_capabilities, sizeof(module_capabilities));
		hon_client.send_message(device_version_request_message, false);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::seconds(2));
	{
		const haier_protocol::HaierMessage device_request_message((uint8_t)FrameType::GET_DEVICE_ID);
		hon_client.send_message(device_request_message, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::seconds(2));
	{
		const haier_protocol::HaierMessage status_request_message((uint8_t)FrameType::CONTROL, (uint16_t)SubcomandsControl::GET_USER_DATA);
		hon_client.send_message(status_request_message, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}
	std::this_thread::sleep_for(std::chrono::seconds(2));
	{
		const haier_protocol::HaierMessage alarm_status_request_message((uint8_t)FrameType::GET_ALARM_STATUS);
		hon_client.send_message(alarm_status_request_message, true);
		hon_client.loop();
		hon_server.loop();
		hon_client.loop();
		hon_server.loop();
	}

}
