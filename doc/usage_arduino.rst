Arduino Usage
=============

This guide explains how to use ``HaierProtocol`` from Arduino IDE/CLI projects.

Install the library
-------------------

Option 1 (Library Manager):

1. Open Arduino IDE.
2. Go to **Sketch -> Include Library -> Manage Libraries...**
3. Search for ``HaierProtocol`` and install.

Option 2 (ZIP/local source):

1. Download the repository as ZIP (or clone it).
2. In Arduino IDE select **Sketch -> Include Library -> Add .ZIP Library...**

Minimal sketch
--------------

Create a sketch like this:

.. code-block:: cpp

  #include <Arduino.h>
  #include <HaierProtocol.h>

  using namespace haier_protocol;

  class UartProtocolStream : public ProtocolStream {
  public:
    explicit UartProtocolStream(Stream& serial) : serial_(serial) {}

    size_t available() noexcept override {
      return static_cast<size_t>(serial_.available());
    }

    size_t read_array(uint8_t* data, size_t len) noexcept override {
      size_t read_count = 0;
      while (read_count < len && serial_.available() > 0) {
        int value = serial_.read();
        if (value < 0) {
          break;
        }
        data[read_count++] = static_cast<uint8_t>(value);
      }
      return read_count;
    }

    void write_array(const uint8_t* data, size_t len) noexcept override {
      serial_.write(data, len);
    }

  private:
    Stream& serial_;
  };

  UartProtocolStream proto_stream(Serial1);
  ProtocolHandler protocol(proto_stream);

  HandlerError on_control(FrameType type, const uint8_t* data, size_t size) {
    (void)data;
    (void)size;

    if (type == FrameType::CONTROL) {
      HaierMessage confirm(FrameType::CONFIRM);
      protocol.send_answer(confirm);
      return HandlerError::HANDLER_OK;
    }
    protocol.no_answer();
    return HandlerError::UNSUPPORTED_MESSAGE;
  }

  void setup() {
    Serial.begin(115200);
    Serial1.begin(9600);

    protocol.set_message_handler(FrameType::CONTROL, on_control);
  }

  void loop() {
    protocol.loop();
  }

Optional request example
------------------------

To send a request expecting answer:

.. code-block:: cpp

  uint8_t data[] = {0x10, 0x20, 0x30};
  HaierMessage request(FrameType::CONTROL, data, sizeof(data));
  protocol.send_message(request, true, 1, std::chrono::milliseconds(300));

If no response is expected:

.. code-block:: cpp

  protocol.send_message_without_answer(request, true);

Include paths
-------------

Both include styles are supported:

- ``#include <HaierProtocol.h>``
- ``#include "protocol/haier_protocol.h"``
