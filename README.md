# HaierProtocol

This library implements a Haier protocol transport level. It can help with sending and receiving messages to appliances that support it and process answer. 

## Protocol description

Haier protocol is a synchronous protocol. All data delivered with this protocol is split into portions - frames. Every frame has the following structure:

<table>
  <tr>
    <td style="text-align: center" colspan=2>separator</td>
    <td style="text-align: center" colspan=7>header</td>
    <td style="text-align: center" colspan=3>message</td>
    <td style="text-align: center" colspan=3>footer</td>
  </tr>
  <tr>
    <td style="text-align: center"><b>byte 1</b></td>
    <td style="text-align: center"><b>byte 2</b></td>
    <td style="text-align: center"><b>byte 3</b></td>
    <td style="text-align: center"><b>byte 4</b></td>
    <td style="text-align: center"><b>byte 5</b></td>
    <td style="text-align: center"><b>byte 6</b></td>
    <td style="text-align: center"><b>byte 7</b></td>
    <td style="text-align: center"><b>byte 8</b></td>
    <td style="text-align: center"><b>byte 10</b></td>
    <td style="text-align: center"><b>byte 11</b></td>
    <td style="text-align: center"><b>...</b></td>
    <td style="text-align: center"><b>byte n</b></td>
    <td style="text-align: center"><b>byte n+1</b></td>
    <td style="text-align: center"><b>byte n+2</b></td>
    <td style="text-align: center"><b>byte n+3</b></td>
  </tr>
  <tr>
    <td style="text-align: center" colspan=2>start frame signature</td>
    <td style="text-align: center">frame size</td>
    <td style="text-align: center">frame flags</td>
    <td style="text-align: center" colspan=4>reserved</td>
    <td style="text-align: center">frame type</td>
    <td style="text-align: center" colspan=3>message data</td>
    <td style="text-align: center">checksum</td>
    <td style="text-align: center" colspan=2>CRC16</td>
  </tr>
</table>

* **start frame signature** - Every frame starts with two bytes with value FF. 
* **frame size** - Size of frame. Don't include the start frame signature and CRC, but include a checksum. Min value 8, the max value is 254.
* **frame flags** - Bit flags byte. The only value supported by this implementation is bit 6 (CRC bit) which indicates if this frame has CRC. So in case if the frame has CRC this byte should be 0x40, if not - 0x00.
* **reserved** - Bytes from 5 to 8 are reserved for future use. They should b 0.
* **frame type** - Type of frame. Supported frame types defined by application level protocol.
* **message data** - Data delivered by frame. In some cases first 2 bytes of data are a subcommand.
* **checksum** - Checksum of the frame, equals to sum of header and message mod 0xFF.
* **CRC16** - If frame flags indicate that frame has CRC this section is equal to CRC for header, message section and checksum

This library implements the transport level of a communication protocol for Haier HVAC units.

Can be used to support both versions of protocols:
  * Smart Air 2: https://www.haierhvac.eu/en/product/residential/tundra-20-0 
  * hOn: https://www.haierairconditioning.nl/innovatieve-technologie/connected-airco/ 

You can find examples in the test folder and also in my custom components repositories for ESPHome:
  * Smart air 2: https://github.com/paveldn/esphome-smartAir2/tree/experimental
  * hOn: https://github.com/paveldn/ESP32-S0WD-Haier/tree/dev

## SAST Tools

[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=github&utm_medium=organic&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
