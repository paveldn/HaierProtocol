HaierProtocol Reference
=======================

This document describes the wire format and runtime behavior implemented in this library.
It is focused on transport and protocol handling logic used by ``HaierProtocol``.

Scope
-----

- This reference describes what is implemented in the current codebase.
- It does not try to redefine vendor-level business commands.
- Frame types are listed in ``src/protocol/haier_frame_types.h``.

Layer model
-----------

The library works in three layers:

1. **Stream layer** (``ProtocolStream``)
2. **Transport/frame layer** (``HaierFrame`` + ``TransportLevelHandler``)
3. **Message/transaction layer** (``HaierMessage`` + ``ProtocolHandler``)

``ProtocolStream`` contract
---------------------------

To use the library, provide an implementation of:

- ``available()``: bytes available for reading
- ``read_array(uint8_t* data, size_t len)``: read up to ``len`` bytes
- ``write_array(const uint8_t* data, size_t len)``: write exactly ``len`` bytes

Frame format
------------

Constants (from ``src/transport/haier_frame.h``):

- ``SEPARATOR_BYTE`` = ``0xFF``
- ``SEPARATOR_POST_BYTE`` = ``0x55``
- ``FRAME_SEPARATORS_COUNT`` = ``2``
- ``FRAME_HEADER_SIZE`` = ``10`` (includes two leading separators)
- ``PURE_HEADER_SIZE`` = ``8`` (header bytes excluding two leading separators)
- ``MAX_FRAME_SIZE`` = ``0xF1`` (241)

Logical frame structure (before byte-stuffing):

.. list-table::
  :header-rows: 1

  * - Field
    - Size (bytes)
    - Notes
  * - Separator
    - 2
    - ``FF FF``
  * - Length
    - 1
    - ``PURE_HEADER_SIZE + data_size``
  * - Flags
    - 1
    - Bit ``0x40`` means CRC present
  * - Reserved
    - 5
    - Zero in generated frames
  * - Type
    - 1
    - Frame type (see ``FrameType``)
  * - Data
    - N
    - 0..233 bytes (effective max with current limits)
  * - Checksum
    - 1
    - 8-bit additive checksum
  * - CRC
    - 2 (optional)
    - CRC-16/ARC when flag ``0x40`` is set

Length and maximum payload
--------------------------

Length byte is validated against:

- minimum: ``PURE_HEADER_SIZE``
- maximum: ``MAX_FRAME_SIZE``

So payload max is:

- ``MAX_FRAME_SIZE - PURE_HEADER_SIZE = 0xF1 - 0x08 = 0xE9`` (233 bytes)

Byte-stuffing
-------------

When serializing a frame, every payload/header/checksum/CRC byte equal to ``0xFF`` is followed by ``0x55``.
This escaping is applied after the first two separator bytes.

On receive, the parser removes escaped ``0x55`` after ``0xFF``. If a byte following ``0xFF`` is not ``0x55`` where escaping is expected, parsing fails with ``WRONG_POST_SEPARATOR_BYTE``.

Checksum and CRC
----------------

Checksum:

- 8-bit additive sum of logical (de-escaped) bytes from ``Length`` through end of ``Data``
- plus ``SEPARATOR_POST_BYTE`` contribution for every stuffed ``0xFF`` byte in header/type/data
- compared to transmitted checksum byte

CRC:

- CRC-16/ARC (polynomial reflected, lookup-table implementation)
- computed from logical bytes ``Length..Data``
- transmitted as two bytes: high byte then low byte
- enabled only if flag bit ``0x40`` is set

Transport-level receive flow
----------------------------

``TransportLevelHandler`` behavior:

- Reads from stream into a circular buffer.
- Searches for frame start by detecting separator pattern.
- Parses header first, then data/checksum/(optional CRC).
- Uses frame timeout ``300 ms`` for partial frames.
- Pushes valid frames into ``incoming_queue_`` as ``TimestampedFrame``.

Error recovery behavior:

- Buffer overflow may drop bytes; partial frame is reset.
- Wrong escaping resets current frame and continues scanning.
- Invalid checksum/CRC drops the frame and continues scanning.

Message model (``HaierMessage``)
--------------------------------

Message payload layout:

- Optional 2-byte subcommand (big-endian)
- Followed by message data bytes

If subcommand is ``NO_SUBCOMMAND`` (``0x0000``), subcommand bytes are omitted.

Protocol transaction behavior (``ProtocolHandler``)
---------------------------------------------------

State machine:

- ``IDLE``
- ``WAITING_FOR_ANSWER``

Defaults:

- max retries constant: ``9`` (request API stores retries as ``min(user, 9) + 1`` attempts)
- default answer timeout: ``200 ms``
- default cooldown between sends: ``400 ms``

Loop algorithm (high-level):

1. Always call transport ``read_data()`` and ``process_data()``.
2. In ``IDLE``:
   - If multiple incoming frames exist, keep only the latest one.
   - Dispatch message handler by incoming frame type.
   - If queued outgoing message exists and timing allows, send it.
3. In ``WAITING_FOR_ANSWER``:
   - On timeout: retry or call timeout handler when retries are exhausted.
   - On incoming frame: dispatch answer handler and finish request.

Handler APIs:

- ``set_message_handler(type, handler)``
- ``set_answer_handler(type, handler)``
- ``set_timeout_handler(type, handler)``
- plus default handlers for each group

Important runtime notes:

- ``send_answer()`` is valid only while a message handler is running.
- ``send_answer(answer)`` (without CRC arg) mirrors CRC mode of the incoming frame.
- ``no_answer()`` can be called in a message handler to suppress warning when intentionally not replying.

Frame type reference
--------------------

See complete enum in:

- ``src/protocol/haier_frame_types.h``

It includes common types such as ``CONTROL``, ``STATUS``, ``CONFIRM``, ``REPORT``, and many query/response pairs.

Minimal integration pattern
---------------------------

1. Implement ``ProtocolStream`` for your UART/serial backend.
2. Construct ``ProtocolHandler`` with that stream.
3. Register message/answer/timeout handlers.
4. Call ``handler.loop()`` frequently from the main loop/task.
5. Use ``send_message(...)`` for request/response transactions.
