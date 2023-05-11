# HaierProtocol
Haier communication protocol library

## General description
Haier protocol has two versions. One is used for older HVAC units that work with the SmartAir2 application. Another is for newer units that work with the hOn application.  Those version commands are different but the frame structure and transport level of protocol are the same. 

## Haier frame
| Frame separator | Frame length | Frame flags | Reserved space | Type | Frame data | Checksum | CRC |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
|  2 bytes | 1 byte | 1 byte | 5 bytes | 1 byte | n bytes (n <= 246) | 1 byte | 2 bytes | 

Where:
* Frame separator - 2 bytes, have fixed value 0xFF 0xFF, used as marker for the beggigning of the frame
* Frame length - 1 byte, number of bytes of the entire frame, includes frame flags, reserved space, type byte, frame data, and checksum, max value is 254
* Frame flags - 1 byte, only 2 values used 0x40 - indicates that frame have CRC bytes, 0x00 - indicates that there is no CRC
* Reserved space - 5 bytes, reserved for future use, fileed with 0x00
* Frame type - 1 byte, type of frame (depend on protocol)
* Frame data - n byte, data of te frame, can be empty. Sometimes first 2 bytes of data used as a subcommand. Max size 246
* Checksum - 1 byte, least significant byte of sum of all bytes bytes of frame except separator bytes, CRC and checksum itself.
* CRC - 2 bytes, CRC 16 of all bytes bytes of frame except separator bytes and CRC itself (modbus CRC algorithm used) 

## SAST Tools

[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=github&utm_medium=organic&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
