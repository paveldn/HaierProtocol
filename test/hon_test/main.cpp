#include <stdint.h>
#include <iostream>
#include <string>
#include "virtual_stream.h"

void main(int argc, char** argv) {
	VirtualStreamHolder stream_holder;
	VirtualStream& stream_a = stream_holder.get_stream_referance(StreamDirection::DIRECTION_A);
	VirtualStream& stream_b = stream_holder.get_stream_referance(StreamDirection::DIRECTION_B);
	{
		uint8_t buf[] { 0x00, 0x01, 0x02, 0x05};
		stream_a.write_array(buf, sizeof(buf));
		std::cout << "Size: " << stream_b.available() << std::endl;
	}
}
