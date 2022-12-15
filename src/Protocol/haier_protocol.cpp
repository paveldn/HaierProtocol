#include <cstring>
#include <chrono>
#include <memory>
#include "Protocol/haier_protocol.h"

#define TAG "haier.protocol"

namespace HaierProtocol
{

constexpr auto MESSAGE_COOLDOWN_INTERVAL = std::chrono::milliseconds(400);
constexpr auto ANSWER_TIMEOUT = std::chrono::milliseconds(150);

ProtocolHandler::ProtocolHandler(ProtocolHandler&& source) noexcept :
	mTransport(std::move(source.mTransport)),
	mMessageHandlersMap(std::move(source.mMessageHandlersMap)),
	mAnswerHandlersMap(std::move(source.mAnswerHandlersMap)),
	mTimeoutHandlersMap(std::move(source.mTimeoutHandlersMap)),
	mOutgoingMessages(std::move(source.mOutgoingMessages)),
	mDefaultMessageHandler(source.mDefaultMessageHandler),
	mDefaultAnswerHandler(source.mDefaultAnswerHandler),
	mDefaultTimeoutHandler(source.mDefaultTimeoutHandler),
	mState(source.mState),
	mProcessingMessage(source.mProcessingMessage),
	mIncommingMessageCrcStatus(source.mIncommingMessageCrcStatus),
	mAnswerSent(source.mAnswerSent),
	mLastMessageType(source.mLastMessageType),
	mCooldownTimeout(source.mCooldownTimeout)
{
	source.mMessageHandlersMap = std::map<uint8_t, MessageHandler>();
	source.mAnswerHandlersMap = std::map<uint8_t, AnswerHandler>();
	source.mTimeoutHandlersMap = std::map<uint8_t, TimeoutHandler>();
}

ProtocolHandler::ProtocolHandler(ProtocolStream& stream) noexcept :
	mTransport(stream),
	mMessageHandlersMap(),
	mAnswerHandlersMap(),
	mTimeoutHandlersMap(),
	mOutgoingMessages(),
	mDefaultMessageHandler(defaultMessageHandler),
	mDefaultAnswerHandler(defaultAnswerHandler),
	mDefaultTimeoutHandler(defaultTimeoutHandler),	
	mState(ProtocolState::psIdle),
	mProcessingMessage(false),
	mIncommingMessageCrcStatus(false),
	mAnswerSent(false),
	mLastMessageType(UNKNOWN_MESSAGE_TYPE)
{
	mCooldownTimeout = std::chrono::steady_clock::time_point();
}

void ProtocolHandler::loop()
{
	mTransport.readData();
	mTransport.processData();
	switch (mState)
	{
	case ProtocolState::psIdle:
		// Check incoming messages
		{
			size_t messagesCount = mTransport.available();
			if (messagesCount > 1)
			{
				// Shouldn't get more than 1 message, drop all except last
				HAIER_LOGW(TAG, "Incoming queue size %d (should be not more than 1). Dropping extra messages", messagesCount);
				mTransport.drop(messagesCount - 1);
				messagesCount = 1;
			}
			if (messagesCount > 0)
			{
				TimestampedFrame frame;
				mTransport.pop(frame);
				uint8_t msgType = frame.frame.getFrameType();
				mIncommingMessageCrcStatus = frame.frame.getUseCrc();
				std::map<uint8_t, MessageHandler>::const_iterator handler = mMessageHandlersMap.find(msgType);
				mProcessingMessage = true;
				mAnswerSent = false;
				if (handler != mMessageHandlersMap.end())
					handler->second(msgType, frame.frame.getData(), frame.frame.getDataSize());
				else
					mDefaultMessageHandler(msgType, frame.frame.getData(), frame.frame.getDataSize());
				mProcessingMessage = false;
				if (!mAnswerSent)
                {
					HAIER_LOGW(TAG, "No answer sent in incoming messages handler, message type %02X", msgType);
                }
			}
			{
				std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
				bool messagesAvailable = !mOutgoingMessages.empty();
				if ((messagesAvailable) && (now >= mCooldownTimeout))
				{
					// Ready to send next message
					OutgoingQueueItem& msg = mOutgoingMessages.front();
					if (writeMessage(msg.first, msg.second))
					{
						mLastMessageType = msg.first.getFrameType();
						mState = ProtocolState::psWaitingWorAnswer;
						mAnswerTimeout = now + ANSWER_TIMEOUT;
					}
					mOutgoingMessages.pop();
				}
			}
		}
		break;
	case ProtocolState::psWaitingWorAnswer:
		// Check for timeout, move to idle after timeout
		if ((std::chrono::steady_clock::now() > mAnswerTimeout))
		{
			std::map<uint8_t, TimeoutHandler>::const_iterator handler = mTimeoutHandlersMap.find(mLastMessageType);
			if (handler != mTimeoutHandlersMap.end())
				handler->second(mLastMessageType);
			else
				mDefaultTimeoutHandler(mLastMessageType);
			mState = ProtocolState::psIdle;
			break;
		}
		if (mTransport.available() > 0)
		{
			TimestampedFrame frame;
			mTransport.pop(frame);
			uint8_t msgType = frame.frame.getFrameType();
			std::map<uint8_t, AnswerHandler>::const_iterator handler = mAnswerHandlersMap.find(mLastMessageType);
			if (handler != mAnswerHandlersMap.end())
				handler->second(mLastMessageType, msgType, frame.frame.getData(), frame.frame.getDataSize());
			else
				mDefaultAnswerHandler(mLastMessageType, msgType, frame.frame.getData(), frame.frame.getDataSize());
			mState = ProtocolState::psIdle;
		}
		break;
	}
}

bool ProtocolHandler::writeMessage(const HaierMessage& message, bool useCrc)
{
	size_t bufSize = message.getBuferSize();
	bool isSuccess = true;
	if (bufSize == 0)
		isSuccess = mTransport.sendData(message.getFrameType(), nullptr, 0, useCrc) > 0;
	else
	{
		std::unique_ptr<uint8_t[]> buffer(new uint8_t[bufSize]);
		isSuccess = (message.fillBuffer(buffer.get(), bufSize) > 0) && (mTransport.sendData(message.getFrameType(), buffer.get(), bufSize, useCrc) > 0);
	}
	if (!isSuccess)
    {
		HAIER_LOGE(TAG, "Error sending message: %02X", message.getFrameType());
    }
	mCooldownTimeout = std::chrono::steady_clock::now() + MESSAGE_COOLDOWN_INTERVAL;
	return isSuccess;
}

void ProtocolHandler::sendMessage(const HaierMessage& message, bool useCrc)
{
	mOutgoingMessages.push(OutgoingQueueItem(message, useCrc));
}

void ProtocolHandler::sendAnswer(const HaierMessage& answer)
{
	if (mProcessingMessage)
	{
		mAnswerSent = writeMessage(answer, mIncommingMessageCrcStatus);
	}
	else
	{
		HAIER_LOGE(TAG, "Answer can be send only from message handler!");
	}
}

void ProtocolHandler::setMessageHandler(uint8_t messageType, MessageHandler handler)
{
	mMessageHandlersMap[messageType] = handler;
}

void ProtocolHandler::removeMessageHandler(uint8_t messageType)
{
	std::map<uint8_t, MessageHandler>::const_iterator it = mMessageHandlersMap.find(messageType);
	if (it != mMessageHandlersMap.end())
		mMessageHandlersMap.erase(it);
}

/// <summary>
/// No way to remove default handler but it is possible to replace it with empty function.
/// </summary>
void ProtocolHandler::setDefaultMessageHandler(MessageHandler handler)
{
	mDefaultMessageHandler = handler;
}

void ProtocolHandler::setAnswerHandler(uint8_t messageType, AnswerHandler handler)
{
	mAnswerHandlersMap[messageType] = handler;
}

void ProtocolHandler::removeAnswerHandler(uint8_t messageType)
{
	std::map<uint8_t, AnswerHandler>::const_iterator it = mAnswerHandlersMap.find(messageType);
	if (it != mAnswerHandlersMap.end())
		mAnswerHandlersMap.erase(it);
}

void ProtocolHandler::setDefaultAnswerHandler(AnswerHandler handler)
{
	mDefaultAnswerHandler = handler;
}

void ProtocolHandler::setTimeoutHandler(uint8_t messageType, TimeoutHandler handler)
{
	mTimeoutHandlersMap[messageType] = handler;
}

void ProtocolHandler::removeTimeoutHandler(uint8_t messageType)
{
	std::map<uint8_t, TimeoutHandler>::const_iterator it = mTimeoutHandlersMap.find(messageType);
	if (it != mTimeoutHandlersMap.end())
		mTimeoutHandlersMap.erase(it);	
}

void ProtocolHandler::setDefaultTimeoutHandler(TimeoutHandler handler)
{
	mDefaultTimeoutHandler = handler;
}

/// <summary>
/// Default message handler, log everything and return heUnsuportedMessage
/// </summary>
/// <param name="messageType">Type of incoming message</param>
/// <param name="data">Incoming message data</param>
/// <param name="dataSize">Size of incoming data</param>
/// <returns>Error code</returns>
HandlerError defaultMessageHandler(uint8_t messageType, const uint8_t* data, size_t dataSize)
{
	HAIER_LOGW(TAG, "Unsupported message received: type %02X data: %s", messageType, dataSize > 0 ? buf2hex(data, dataSize).c_str() : "<empty>");
	return HandlerError::heUnsuportedMessage;
}

/// <summary>
/// Default message handler, log everything and return heUnsuportedMessage
/// </summary>
/// <param name="requestType">Request that caused this answer</param>
/// <param name="messageType">Type of incoming message</param>
/// <param name="data">Incoming message data</param>
/// <param name="dataSize">Size of incoming data</param>
/// <returns>Error code</returns>
HandlerError defaultAnswerHandler(uint8_t requestType, uint8_t messageType, const uint8_t* data, size_t dataSize)
{
	HAIER_LOGW(TAG, "Unsupported answer to %02X received: type %02X data: %s", requestType, messageType, dataSize > 0 ? buf2hex(data, dataSize).c_str() : "<empty>");
	return HandlerError::heUnsuportedMessage;
}

/// <summary>
/// Default message handler, log everything and return heUnsuportedMessage
/// </summary>
/// <param name="requestType">Request that caused timeout</param>
/// <returns>Error code</returns>
HandlerError defaultTimeoutHandler(uint8_t messageType)
{
	HAIER_LOGW(TAG, "Message %02X answer timeout", messageType);
	return HandlerError::heOK;
}


} // HaierProtocol