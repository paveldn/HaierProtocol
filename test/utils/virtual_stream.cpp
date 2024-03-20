#include "virtual_stream.h"

#define BUFFER_SIZE 4096

VirtualStream::VirtualStream(CircularBuffer<uint8_t>& tx_buffer, CircularBuffer<uint8_t>& rx_buffer) :
	tx_buffer_(tx_buffer),
	rx_buffer_(rx_buffer) {
}

size_t VirtualStream::available() noexcept {
	return this->rx_buffer_.get_size();
}
size_t VirtualStream::read_array(uint8_t* data, size_t len) noexcept {
	return this->rx_buffer_.pop(data, len);
}
void VirtualStream::write_array(const uint8_t* data, size_t len) noexcept {
	this->tx_buffer_.push(data, len);
}

VirtualStreamHolder::VirtualStreamHolder() : 
	buffers_{ CircularBuffer<uint8_t>(BUFFER_SIZE), CircularBuffer<uint8_t>(BUFFER_SIZE) },
	streams_{ VirtualStream(buffers_[0], buffers_[1]), VirtualStream(buffers_[1], buffers_[0]) } {
}

VirtualStream& VirtualStreamHolder::get_stream_reference(StreamDirection direction) {
	if (direction == StreamDirection::DIRECTION_A)
		return this->streams_[0];
	return this->streams_[1];
}
