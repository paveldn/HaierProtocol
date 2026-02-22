HaierProtocol
=============

This library implements a Haier protocol transport level. It can help
with sending and receiving messages to appliances that support it and
process answers.

Compatibility
-------------

This library is compatible with both **PlatformIO** and **Arduino** toolchains.

- Existing include paths stay unchanged (for example ``"protocol/haier_protocol.h"``).
- Public headers are located in the ``src`` tree (the ``include`` tree is no longer used).
- For Arduino IDE/CLI, you can also include the main header:

Note on C++ standard library support
----------------------------------

This library uses C++ timing facilities (``std::chrono``). It requires a
toolchain and board core that provide the C++ standard library. Older or
constrained Arduino cores (for example some AVR/Uno toolchains) may not
provide ``<chrono>`` and will fail to compile. For these boards prefer
PlatformIO or newer Arduino cores that include full C++ support.

If you target constrained boards, see the Arduino usage documentation for
workarounds and recommended cores.

- For Arduino IDE/CLI, you can also include the main header:

  .. code-block:: cpp

    #include <HaierProtocol.h>

No migration is required for existing PlatformIO projects.

Documentation
-------------

- `Protocol reference <doc/protocol_reference.rst>`_
- `PlatformIO usage example <doc/usage_platformio.rst>`_
- `Arduino usage example <doc/usage_arduino.rst>`_
- `hOn simulator <doc/hon_simulator.rst>`_
- `SmartAir2 simulator <doc/smartair2_simulator.rst>`_

Protocol description
--------------------

Detailed reference: `HaierProtocol Reference <doc/protocol_reference.rst>`_

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
 * **CRC** - 2 bytes, CRC 16 of all bytes of the frame except separator bytes, checksum byte, and CRC itself (`CRC-16/ARC <https://crccalc.com/?crc=&method=CRC-16/ARC&datatype=1&outtype=0>`_ algorithm used). CRC is available only if the frame flags byte indicates it.
