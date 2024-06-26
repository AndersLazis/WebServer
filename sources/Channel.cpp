#include "Channel.hpp"

Channel::Channel()
       : sender(NULL), receiver(NULL), handler(NULL),
			   log(Logger::instance(), "Channel")
{
}

Channel::~Channel()
{
	if (this->receiver && (dynamic_cast<RequestReceiver *>(this->receiver)) && this->handler)
	{
		delete this->handler;
		this->handler = NULL;
	}
	if (this->sender)
	{
		delete this->sender;
		this->sender = NULL;
	}
	if (this->receiver)
	{
		delete this->receiver;
		this->receiver = NULL;
	}
}

Channel::Channel(const Channel &other)
       : log(Logger::instance(), "Channel")
{
	*this = other;
}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		this->sender = other.sender;
		this->receiver = other.receiver;
		this->handler = other.handler;
		this->log = other.log;
	}
	return *this;
}



void Channel::setSender(ISender *sender)
{
	this->sender = sender;
}

void Channel::setReceiver(IReceiver *receiver)
{
	this->receiver = receiver;
}

void Channel::setHandler(IHandler *handler)
{
	if (!handler)
		return ;
	if (this->handler)
		delete this->handler;
	this->handler = handler;
}



IReceiver *Channel::getReceiver()
{
	return this->receiver;
}

ISender *Channel::getSender()
{
	return this->sender;
}

IHandler *Channel::getHandler()
{
	return this->handler;
}

// Public

void Channel::receive()
{
	this->receiver->consume();
}

void Channel::send()
{
	if (this->sender && this->sender->readyToSend())
		this->sender->sendData();
}

bool Channel::senderFinished()
{
	return this->sender->finished();
}
