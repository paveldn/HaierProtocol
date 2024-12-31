#pragma once

#include "protocol/haier_protocol.h"

class HaierBaseServer {
public:
	HaierBaseServer() = delete;
	HaierBaseServer(haier_protocol::ProtocolStream&);
	virtual ~HaierBaseServer();
	virtual void loop();
protected:
    virtual void register_handlers_() = 0;
    haier_protocol::ProtocolHandler* protocol_handler_;
};
