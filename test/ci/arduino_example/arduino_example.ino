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
