#include <stdint.h>
#include "Utils/circular_buffer.h"
#include "Utils/protocol_stream.h"
#include "Utils/haier_log.h"
#include "Transport/protocol_transport.h"
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
