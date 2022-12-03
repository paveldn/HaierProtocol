#ifndef HAIRER_PROTOCOL_H
#define HAIRER_PROTOCOL_H

#include <cstdint>
#include <chrono>
#include <functional>
#include <map>
#include <queue>
#include "Transport/protocol_transport.h"
#include "haier_message.h"

namespace HaierProtocol
{

enum class HandlerError
{
	heOK = 0,
	heUnsuportedMessage,
	heUnexpectedMessage,
	heUnsupportedSubcommand,
	heWrongMessageStructure,
	heRuntimeError,
	heUnknownError,
	NUM_HANDLER_ERROR
};

// Message handler type. Expected that function sends answer back.
// argument 1: Incoming message type
// argument 2: Incoming data buffer (nullptr if none)
// argument 3: Incoming data buffer size
// return: Result of processing
#define MessageHandler std::function<HandlerError(uint8_t, const uint8_t*, size_t)>

// Answers handler type.
// argument 1: Request message type that caused this answer
// argument 2: Incoming message type
// argument 3: Incoming data buffer (nullptr if none)
// argument 4: Incoming data buffer size
// return: Result of processing
#define AnswerHandler std::function<HandlerError(uint8_t, uint8_t, const uint8_t*, size_t)>

// Timeout handler type.
// argument 1: Request message type that caused this answer
// return: Result of processing
#define TimeoutHandler std::function<HandlerError(uint8_t)>

HandlerError defaultMessageHandler(uint8_t messageType, const uint8_t* data, size_t dataSize);
HandlerError defaultAnswerHandler(uint8_t messageType, uint8_t requestType, const uint8_t* data, size_t dataSize);
HandlerError defaultTimeoutHandler(uint8_t messageType);

class ProtocolHandler 
{
public:

	ProtocolHandler() = delete;
	ProtocolHandler(const ProtocolHandler&) = delete;
	ProtocolHandler& operator=(const ProtocolHandler&) = delete;
	ProtocolHandler(ProtocolHandler&&) noexcept;
	explicit ProtocolHandler(ProtocolStream&) noexcept;
	size_t getOutgoingQueueSize() const noexcept {return mOutgoingMessages.size(); };
	void sendMessage(const HaierMessage& command, bool useCrc);
	void sendAnswer(const HaierMessage& answer);
	void setMessageHandler(uint8_t messageType, MessageHandler handler);
	void removeMessageHandler(uint8_t messageType);
	void setDefaultMessageHandler(MessageHandler handler);
	void setAnswerHandler(uint8_t messageType, AnswerHandler handler);
	void removeAnswerHandler(uint8_t messageType);
	void setDefaultAnswerHandler(AnswerHandler handler);
	void setTimeoutHandler(uint8_t messageType, TimeoutHandler handler);
	void removeTimeoutHandler(uint8_t messageType);
	void setDefaultTimeoutHandler(TimeoutHandler handler);
	virtual void loop();
protected:
	bool writeMessage(const HaierMessage& message, bool useCrc);
private:
	enum class ProtocolState
	{
		psIdle,
		psWaitingWorAnswer,
		psAfterPacketDelay,
	};
	typedef std::pair<const HaierMessage, bool>	OutgoingQueueItem;
	typedef std::queue<OutgoingQueueItem>	OutgoingQueue;
	TransportLevelHandler					mTransport;
	std::map<uint8_t, MessageHandler>		mMessageHandlersMap;
	std::map<uint8_t, AnswerHandler>		mAnswerHandlersMap;
	std::map<uint8_t, TimeoutHandler>		mTimeoutHandlersMap;	
	OutgoingQueue							mOutgoingMessages;
	MessageHandler							mDefaultMessageHandler;
	AnswerHandler							mDefaultAnswerHandler;
	TimeoutHandler							mDefaultTimeoutHandler;
	ProtocolState							mState;
	bool									mProcessingMessage;
	bool									mIncommingMessageCrcStatus;
	bool									mAnswerSent;
	uint8_t									mLastMessageType;
	std::chrono::steady_clock::time_point	mCooldownTimeout;
	std::chrono::steady_clock::time_point	mAnswerTimeout;
};


} // HaierProtocol
#endif // HAIRER_PROTOCOL_H