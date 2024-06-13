#ifndef HAIER_FRAME_TYPES_H
#define HAIER_FRAME_TYPES_H

#include <stdint.h>

namespace haier_protocol
{


// In this section comments:
//  - module is the ESP32 control module (communication module in Haier protocol document)
//  - device is the conditioner control board (network appliances in Haier protocol document)
enum class FrameType : uint8_t {
  UNKNOWN_FRAME_TYPE = 0x00,
  CONTROL = 0x01,  // Requests or sets one or multiple parameters (module <-> device, required)
  STATUS = 0x02,   // Contains one or multiple parameters values, usually answer to control frame (module <-> device, required)
  INVALID = 0x03,  // Communication error indication (module <-> device, required)
  ALARM_STATUS = 0x04,  // Alarm status report (module <-> device, interactive, required)
  CONFIRM = 0x05,  // Acknowledgment, usually used to confirm reception of frame if there is no special answer (module <-> device, required)
  REPORT = 0x06,   // Report frame (module <-> device, interactive, required)
  STOP_FAULT_ALARM = 0x09,             // Stop fault alarm frame (module -> device, interactive, required)
  SYSTEM_DOWNLINK = 0x11,              // System downlink frame (module -> device, optional)
  DEVICE_UPLINK = 0x12,                // Device uplink frame (module <- device , interactive, optional)
  SYSTEM_QUERY = 0x13,                 // System query frame (module -> device, optional)
  SYSTEM_QUERY_RESPONSE = 0x14,        // System query response frame (module <- device , optional)
  DEVICE_QUERY = 0x15,                 // Device query frame (module <- device, optional)
  DEVICE_QUERY_RESPONSE = 0x16,        // Device query response frame (module -> device, optional)
  GROUP_COMMAND = 0x60,                // Group command frame (module -> device, interactive, optional)
  GET_DEVICE_VERSION = 0x61,           // Requests device version (module -> device, required)
  GET_DEVICE_VERSION_RESPONSE = 0x62,  // Device version answer (module <- device, required_
  GET_ALL_ADDRESSES = 0x67,            // Requests all devices addresses (module -> device, interactive, optional)
  GET_ALL_ADDRESSES_RESPONSE = 0x68,   // Answer to request of all devices addresses (module <- device , interactive, optional)
  HANDSET_CHANGE_NOTIFICATION = 0x69,  // Handset change notification frame (module <- device , interactive, optional)
  GET_DEVICE_ID = 0x70,                // Requests Device ID (module -> device, required)
  GET_DEVICE_ID_RESPONSE = 0x71,       // Response to device ID request (module <- device , required)
  GET_ALARM_STATUS = 0x73,             // Alarm status request (module -> device, required)
  GET_ALARM_STATUS_RESPONSE = 0x74,    // Response to alarm status request (module <- device, required)
  GET_DEVICE_CONFIGURATION = 0x7C,     // Requests device configuration (module -> device, interactive, required)
  GET_DEVICE_CONFIGURATION_RESPONSE = 0x7D,  // Response to device configuration request (module <- device, interactive, required)
  DOWNLINK_TRANSPARENT_TRANSMISSION = 0x8C,  // Downlink transparent transmission (proxy data Haier cloud -> device) (module -> device, interactive, optional)
  UPLINK_TRANSPARENT_TRANSMISSION = 0x8D,    // Uplink transparent transmission (proxy data device -> Haier cloud) (module <- device, interactive, optional)
  START_DEVICE_UPGRADE = 0xE1,               // Initiate device OTA upgrade (module -> device, OTA required)
  START_DEVICE_UPGRADE_RESPONSE = 0xE2,      // Response to initiate device upgrade command (module <- device, OTA required)
  GET_FIRMWARE_CONTENT = 0xE5,               // Requests to send firmware (module <- device, OTA required)
  GET_FIRMWARE_CONTENT_RESPONSE = 0xE6,      // Response to send firmware request (module -> device, OTA required) (multipacket?)
  CHANGE_BAUD_RATE = 0xE7,                   // Requests to change port baud rate (module <- device, OTA required)
  CHANGE_BAUD_RATE_RESPONSE = 0xE8,          // Response to change port baud rate request (module -> device, OTA required)
  GET_SUBBOARD_INFO = 0xE9,                  // Requests subboard information (module -> device, required)
  GET_SUBBOARD_INFO_RESPONSE = 0xEA,         // Response to subboard information request (module <- device, required)
  GET_HARDWARE_INFO = 0xEB,                  // Requests information about device and subboard (module -> device, required)
  GET_HARDWARE_INFO_RESPONSE = 0xEC,         // Response to hardware information request (module <- device, required)
  GET_UPGRADE_RESULT = 0xED,                 // Requests result of the firmware update (module <- device, OTA required)
  GET_UPGRADE_RESULT_RESPONSE = 0xEF,        // Response to firmware update results request (module -> device, OTA required)
  GET_NETWORK_STATUS = 0xF0,                 // Requests network status (module <- device, interactive, optional)
  GET_NETWORK_STATUS_RESPONSE = 0xF1,        // Response to network status request (module -> device, interactive, optional)
  START_WIFI_CONFIGURATION = 0xF2,           // Starts WiFi configuration procedure (module <- device, interactive, required)
  START_WIFI_CONFIGURATION_RESPONSE = 0xF3,  // Response to start WiFi configuration request (module -> device, interactive, required)
  STOP_WIFI_CONFIGURATION = 0xF4,            // Stop WiFi configuration procedure (module <- device, interactive, required)
  STOP_WIFI_CONFIGURATION_RESPONSE = 0xF5,   // Response to stop WiFi configuration request (module -> device, interactive, required)
  REPORT_NETWORK_STATUS = 0xF7,              // Reports network status (module -> device, required)
  CLEAR_CONFIGURATION = 0xF8,                // Request to clear module configuration (module <- device, interactive, optional)
  BIG_DATA_REPORT_CONFIGURATION = 0xFA,      // Configuration for auto report device full status (module -> device, interactive, optional)
  BIG_DATA_REPORT_CONFIGURATION_RESPONSE = 0xFB,  // Response to set big data configuration (module <- device, interactive, optional)
  GET_MANAGEMENT_INFORMATION = 0xFC,              // Request management information from device (module -> device, required)
  GET_MANAGEMENT_INFORMATION_RESPONSE = 0xFD,     // Response to management information request (module <- device, required)
  WAKE_UP = 0xFE,                                 // Request to wake up (module <-> device, optional)
};

} // HaierProtocol
#endif // HAIER_FRAME_TYPES_H
