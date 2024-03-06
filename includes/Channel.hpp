#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include "CGIReceiver.hpp"
# include "RequestReceiver.hpp"
# include "handlers/CGIHandler.hpp"
# include "interfaces/ISender.hpp"
# include "interfaces/IReceiver.hpp"
# include "interfaces/IHandler.hpp"
# include "interfaces/IChannel.hpp"

#include "logger/LoggerIncludes.hpp"

class Channel: IChannel
{
	private:
		ISender		*sender;
		IReceiver	*receiver;
		IHandler	*handler;
		Logger		log;
	public:
		Channel();
		~Channel();
		Channel(const Channel &other);
		Channel &operator=(const Channel &other);
		
		void		setSender(ISender *sender);
		void		setReceiver(IReceiver *receiver);
		void		setHandler(IHandler *handler);
		
		IReceiver	*getReceiver();
		ISender		*getSender();
		IHandler	*getHandler();
		// Public
		void		receive();
		void		send();
		bool		senderFinished();
};
#endif