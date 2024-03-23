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
#include "test_macro.h"

using namespace esphome::haier::smartair2_protocol;

HaierPacketControl ac_full_state;

haier_protocol::FrameType expected_answers[][2] = {
	{haier_protocol::FrameType::GET_DEVICE_VERSION, haier_protocol::FrameType::GET_DEVICE_VERSION_RESPONSE},
	{haier_protocol::FrameType::CONTROL, haier_protocol::FrameType::STATUS},
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
	haier_protocol::ProtocolHandler smartair2_server(server_stream);
	smartair2_server.set_message_handler(haier_protocol::FrameType::GET_DEVICE_VERSION, std::bind(unsupported_message_handler, &smartair2_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	smartair2_server.set_message_handler(haier_protocol::FrameType::CONTROL, std::bind(status_request_handler, &smartair2_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	smartair2_server.set_message_handler(haier_protocol::FrameType::REPORT_NETWORK_STATUS, std::bind(report_network_status_handler, &smartair2_server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	haier_protocol::ProtocolHandler smartair2_client(client_stream);
	ac_full_state = get_ac_state_ref();
	smartair2_client.set_default_answer_handler(client_answers_handler);
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST1)
	{
		TEST_START(1);
		uint8_t module_capabilities[2] = { 0b00000000, 0b00000111 };
		const haier_protocol::HaierMessage device_version_request_message(haier_protocol::FrameType::GET_DEVICE_VERSION, module_capabilities, sizeof(module_capabilities));
		smartair2_client.send_message(device_version_request_message, false, 3);
		smartair2_client.loop();
		smartair2_server.loop();
		smartair2_client.loop();
		smartair2_server.loop();
		for (int i = 0; i <= 2; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			smartair2_client.loop();
			smartair2_server.loop();
		}
		TEST_END(2, 0);
	}
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST2)
	{
		// Unsupported message
		TEST_START(2);
		constexpr size_t ATTEMPTS_NUM = 3;
		const haier_protocol::HaierMessage unsupported_request_message(haier_protocol::FrameType::DOWNLINK_TRANSPARENT_TRANSMISSION);
		smartair2_client.send_message(unsupported_request_message, false, ATTEMPTS_NUM - 1);
		smartair2_client.loop();
		smartair2_server.loop();
		smartair2_client.loop();
		smartair2_server.loop();
		for (int i = 0; i <= 2 * ATTEMPTS_NUM; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			smartair2_client.loop();
			smartair2_server.loop();
		}
		TEST_END(2 * ATTEMPTS_NUM + 1, 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST3)
	{
		TEST_START(3);
		const haier_protocol::HaierMessage status_request_message(haier_protocol::FrameType::CONTROL, 0x4D01);
		smartair2_client.send_message(status_request_message, false);
		smartair2_client.loop();
		smartair2_server.loop();
		smartair2_client.loop();
		smartair2_server.loop();
		TEST_END(0, 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST4)
	{
		TEST_START(4);
		const uint8_t wifi_status_data[4] = { 0x00, 0x01, 0x00, 0x37 };
		haier_protocol::HaierMessage wifi_status_request(haier_protocol::FrameType::REPORT_NETWORK_STATUS, wifi_status_data, sizeof(wifi_status_data));
		smartair2_client.send_message(wifi_status_request, false);
		smartair2_client.loop();
		smartair2_server.loop();
		smartair2_client.loop();
		smartair2_server.loop();
		TEST_END(0, 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
#endif
#if defined(RUN_ALL_TESTS) || defined(RUN_TEST5)
	{
		TEST_START(5);
		ac_full_state.ac_power = true;
		ac_full_state.ac_mode = (uint8_t)ConditioningMode::COOL;
		ac_full_state.display_status = 0;
		haier_protocol::HaierMessage control_message(haier_protocol::FrameType::CONTROL, 0x4D5F, (uint8_t*)&ac_full_state, sizeof(HaierPacketControl));
		smartair2_client.send_message(control_message, false);
		smartair2_client.loop();
		smartair2_server.loop();
		smartair2_client.loop();
		smartair2_server.loop();
		if (memcmp(&ac_full_state, &get_ac_state_ref(), sizeof(HaierPacketControl)) == 0) {
			HAIER_LOGI("AC control processed correctly");
		}
		else {
			HAIER_LOGW("AC control not OK");
		}
		TEST_END(0, 0);
	}
#endif
	HAIER_LOGI("All tests successfully finished!");
}

