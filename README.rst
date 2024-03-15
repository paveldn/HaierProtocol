HaierProtocol
=============

This library implements a Haier protocol transport level. It can help
with sending and receiving messages to appliances that support it and
process answers.

Protocol description
--------------------

Haier protocol is a synchronous protocol. All data delivered with this
protocol is split into portions - frames. The protocol has two versions.
One is used for older HVAC units that work with the SmartAir2
application. Another is for newer units that work with the hOn
application. Those version commands are different but the frame
structure and transport level of the protocol are the same.

Haier frame
-----------

.. list-table:: Frame structure

  * - **Frame separator**
    - **Frame length**
    - **Frame flags**
    - **Reserved space**
    - **Type**
    - **Frame data**
    - **Checksum**
    - **CRC**
  * - 2 bytes
    - 1 byte
    - 1 byte
    - 5 bytes
    - 1 byte
    - | n bytes
      | (n <= 246)
    - 1 byte
    - 2 bytes 

Where:
 * **Frame separator** - 2 bytes, have fixed value 0xFF 0xFF, used as a marker for the beginning of the frame
 * **Frame length** - 1 byte, number of bytes of the entire frame, includes frame flags, reserved space, type byte, frame data, and checksum, max value is 254 \*
 * **Frame flags** - 1 byte, only 2 values used 0x40 - indicates that frame have CRC bytes, 0x00 - indicates that there is no CRC
 * **Reserved space** - 5 bytes, reserved for future use, filled with 0x00
 * **Frame type** - 1 byte, type of frame (depend on the protocol)
 * **Frame data** - n byte, data of the frame, can be empty. Sometimes first 2 bytes of data are used as a subcommand. Max size 246
 * **Checksum** - 1 byte, the least significant byte of the sum of all bytes of the frame except separator bytes, CRC, and checksum itself.
 * **CRC** - 2 bytes, CRC 16 of all bytes of the frame except separator bytes and CRC itself (Modbus CRC algorithm used). CRC is available only if the frame flags byte indicates it.
