# HaierProtocol
Haier communication protocol library

## General description
Haier protocol has two versions. One is used for older HVAC units that work with the SmartAir2 application. Another is for newer units that work with the hOn application.  Those version commands are different but the frame structure and transport level of protocol are the same. 

## Haier frame

This library implements the transport level of a communication protocol for Haier HVAC units. 

Can be used to support both versions of protocols:
  * Smart Air 2: https://www.haierhvac.eu/en/product/residential/tundra-20-0 
  * hOn: https://www.haierairconditioning.nl/innovatieve-technologie/connected-airco/ 

You can find examples in the test folder and also in my custom components repositories for ESPHome:
  * Smart air 2: https://github.com/paveldn/esphome-smartAir2/tree/experimental
  * hOn: https://github.com/paveldn/ESP32-S0WD-Haier/tree/dev

## SAST Tools

[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=github&utm_medium=organic&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
