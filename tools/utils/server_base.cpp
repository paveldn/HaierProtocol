#include <cassert>
#include "server_base.h"
#include "protocol/haier_protocol.h"

HaierBaseServer::HaierBaseServer(haier_protocol::ProtocolStream& stream) :
	protocol_handler_(new haier_protocol::ProtocolHandler(stream)) 
{
}

HaierBaseServer::~HaierBaseServer() {
    delete this->protocol_handler_;
}

void HaierBaseServer::loop() {
	static bool first_run = true;
	if (first_run) {
		this->register_handlers_();
		first_run = false;
	}
	this->protocol_handler_->loop();
}
