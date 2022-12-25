#include <stdint.h>
#include "utils/circular_buffer.h"
#include "utils/protocol_stream.h"
#include "utils/haier_log.h"
#include "transport/protocol_transport.h"
#include "console_log.h"
#include <iostream>

void main(int argc, char** argv)
{
	if (argc == 1)
	{

	}
	else
	{
		std::cout << "Please use: hon_test <port>" << std::endl;
	}
}
