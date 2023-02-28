#include <stdint.h>
#include "utils/haier_log.h"
#include "protocol/haier_protocol.h"
#include "console_log.h"
#include "serial_stream.h"  
#include <iostream>
#include <string>

#define CONTROL 0x01
#define STATUS 0x02
#define INVALID 0x03
#define ALARM_STATUS 0x04
#define CONFIRM 0x05
#define GET_DEVICE_VERSION 0x61
#define GET_DEVICE_VERSION_RESPONSE 0x62
#define GET_DEVICE_ID 0x70
#define GET_DEVICE_ID_RESPONSE 0x71
#define GET_ALARM_STATUS 0x73
#define GET_ALARM_STATUS_RESPONSE 0x74
#define REPORT_NETWORK_STATUS 0xF7
#define GET_MANAGEMENT_INFORMATION 0xFC
#define GET_MANAGEMENT_INFORMATION_RESPONSE 0xFD

haier_protocol::HandlerError get_device_version_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
    if (type == GET_DEVICE_VERSION) {
        if ((size == 2) && (buffer[0] == 0x00) && (buffer[1] == 0x07)) {
            uint8_t device_version_buf[] = { 0x45, 0x2B, 0x2B, 0x32, 0x2E, 0x31, 0x38, 0x00, 0x31, 0x37,
                                             0x30, 0x36, 0x32, 0x36, 0x30, 0x30, 0xF1, 0x00, 0x00, 0x31,
                                             0x37, 0x30, 0x35, 0x32, 0x36, 0x30, 0x30, 0x01, 0x55, 0x2D,
                                             0x41, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x5B };
            protocol_handler->send_answer(haier_protocol::HaierMessage(GET_DEVICE_VERSION_RESPONSE, device_version_buf, sizeof(device_version_buf)), true);
            return haier_protocol::HandlerError::HANDLER_OK;
        } else
            return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    } else
        return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
}

haier_protocol::HandlerError get_device_id_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
    if (type == GET_DEVICE_ID) {
        if (size == 0) {
            uint8_t device_id_buf[] = { 0x45, 0x2B, 0x2B, 0x32, 0x2E, 0x31, 0x38, 0x00, 0x31, 0x37,
                                        0x30, 0x36, 0x32, 0x36, 0x30, 0x30, 0xF1, 0x00, 0x00, 0x31,
                                        0x37, 0x30, 0x35, 0x32, 0x36, 0x30, 0x30, 0x01, 0x55, 0x2D,
                                        0x41, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };
            protocol_handler->send_answer(haier_protocol::HaierMessage(GET_DEVICE_ID_RESPONSE, device_id_buf, sizeof(device_id_buf)), true);
            return haier_protocol::HandlerError::HANDLER_OK;
        } else
            return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    } else
        return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
}

haier_protocol::HandlerError status_request_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
    if (type == CONTROL) {
        static uint8_t state_buffer[] = { 0x07, 0x00, 0x85, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x22, 0x00, 0x42, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00 };
        if ((size == 2) && (buffer[0] == 0x4D) && (buffer[1] == 0x01)) {
            protocol_handler->send_answer(haier_protocol::HaierMessage(STATUS, 0x6D01, state_buffer, sizeof(state_buffer)));
            return haier_protocol::HandlerError::HANDLER_OK;
        } else if ((size > 2) && (buffer[0] == 0x60) && (buffer[1] == 0x01)) {
            memcpy(state_buffer, &buffer[2], min(size - 2, sizeof(state_buffer)));
            protocol_handler->send_answer(haier_protocol::HaierMessage(STATUS, 0x6D5F, state_buffer, sizeof(state_buffer)));
            return haier_protocol::HandlerError::HANDLER_OK;
        } else
            return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    } else
        return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
}

haier_protocol::HandlerError alarm_status_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
    if (type == GET_ALARM_STATUS) {
        if (size == 0) {
            uint8_t alarm_status_buf[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            protocol_handler->send_answer(haier_protocol::HaierMessage(GET_ALARM_STATUS_RESPONSE, 0x0F5A, alarm_status_buf, sizeof(alarm_status_buf)));
            return haier_protocol::HandlerError::HANDLER_OK;
        } else
            return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    } else
        return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
}

haier_protocol::HandlerError get_managment_information_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
    if (type == GET_MANAGEMENT_INFORMATION) {
        if (size == 0) {
            uint8_t managment_information_buf[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            protocol_handler->send_answer(haier_protocol::HaierMessage(GET_MANAGEMENT_INFORMATION_RESPONSE, managment_information_buf, sizeof(managment_information_buf)));
            return haier_protocol::HandlerError::HANDLER_OK;
        } else
            return haier_protocol::HandlerError::UNSUPPORTED_SUBCOMMAND;
    } else
        return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
}

haier_protocol::HandlerError report_network_status_handler(haier_protocol::ProtocolHandler* protocol_handler, uint8_t type, const uint8_t* buffer, size_t size) {
    if (type == REPORT_NETWORK_STATUS) {
        protocol_handler->send_answer(haier_protocol::HaierMessage(CONFIRM));
        return haier_protocol::HandlerError::HANDLER_OK;
    } else
        return haier_protocol::HandlerError::UNSUPORTED_MESSAGE;
}

void main(int argc, char** argv) {
  if (argc == 2) {
    HWND console_wnd;
    console_wnd = GetForegroundWindow();
    haier_protocol::set_log_handler(console_logger);
    SerailStream serial_stream(std::string("\\\\.\\").append(argv[1]).c_str());
    if (!serial_stream.is_valid()) {
        std::cout << "Can't open port " << argv[1] << std::endl;
        return;
    }
    haier_protocol::ProtocolHandler hon_handler(serial_stream);
    hon_handler.set_message_handler(GET_DEVICE_VERSION, std::bind(&get_device_version_handler, &hon_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    hon_handler.set_message_handler(GET_DEVICE_ID, std::bind(&get_device_id_handler, &hon_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    hon_handler.set_message_handler(CONTROL, std::bind(&status_request_handler, &hon_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    hon_handler.set_message_handler(GET_ALARM_STATUS, std::bind(&alarm_status_handler, &hon_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    hon_handler.set_message_handler(GET_MANAGEMENT_INFORMATION, std::bind(&get_managment_information_handler, &hon_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    hon_handler.set_message_handler(REPORT_NETWORK_STATUS, std::bind(&report_network_status_handler, &hon_handler, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    SetConsoleTitle("hOn HVAC simulator, press ESC to exit");
    while ((console_wnd != GetForegroundWindow()) || ((GetKeyState(VK_ESCAPE) & 0x8000) == 0)) {
        hon_handler.loop();
        Sleep(20);
    }
  } else {
    std::cout << "Please use: hon_simulator <port>" << std::endl;
  }
}
