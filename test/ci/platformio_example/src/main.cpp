#include <Arduino.h>
#include <HaierProtocol.h>

using namespace haier_protocol;

class UartProtocolStream : public ProtocolStream {
public:
  explicit UartProtocolStream(HardwareSerial& serial) : serial_(serial) {}

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
    serial_.flush();
  }

private:
  HardwareSerial& serial_;
};

UartProtocolStream proto_stream(Serial2);
ProtocolHandler protocol(proto_stream);

HandlerError on_message(FrameType type, const uint8_t* data, size_t size) {
  if (type == FrameType::CONTROL) {
    HaierMessage answer(FrameType::CONFIRM);
    protocol.send_answer(answer);
    return HandlerError::HANDLER_OK;
  }
  protocol.no_answer();
  return HandlerError::UNSUPPORTED_MESSAGE;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  protocol.set_message_handler(FrameType::CONTROL, on_message);
  protocol.set_answer_timeout(200);
  protocol.set_cooldown_interval(400);
}

void loop() {
  protocol.loop();

  // Example outgoing request every 5 seconds
  static uint32_t last_send = 0;
  uint32_t now = millis();
  if (now - last_send > 5000) {
    uint8_t payload[] = {0x01, 0x02};
    HaierMessage msg(FrameType::CONTROL, payload, sizeof(payload));
    protocol.send_message(msg, true, 1, std::chrono::milliseconds(300));
    last_send = now;
  }
}
