#ifndef MANAGER_HPP
# define MANAGER_HPP
# include "Connection.hpp"
# include "ConfigParser.hpp"
# include <sys/epoll.h>

#include "logger/LoggerIncludes.hpp"

class Manager
{
	public:
		enum EventType
		{
			NEW_CONN,
			READ_AVAIL,
			WRITE_AVAIL,
			READWRITE_AVAIL,
			EOF_CONN
		};

		static bool			running;
		static void			signalHandlerSigint(int sigNum);

		Manager(std::vector<Server> servers, char **ev);
		~Manager();

		void						startServer();

		//epoll
		void						addSocketToEpoll(int socketFd);
		void						setEpollReadWriteFlags(int socketFd);
		void						setEpollReadFlag(int socket);
		void						removeSocketFromEpoll(int socket);


		void						addConnection(int socket, Connection *conn);
		void						removeConnection(int socket);

	private:
		static const int						maxEpollEvents = 128;
		struct epoll_event					epollEvents[Manager::maxEpollEvents];
		static const int 						maxNetworkErrorRetries = 3;

		std::vector<Server> 				servers; //from parser
		std::map<int,
		         Connection*>				connections;
		Logger											log;
		int													epollFd;
    char**											ev;

		Manager();
		Manager(const Manager &other);
		Manager& operator=(const Manager &other);

		void									run();
	
		//epoll
		void									initEpoll();
		void									addServerSocketToEpoll(int socketFd);
		int										getNewEpollEventsCount();
		int										getEventFd(int eventIndex);
		Manager::EventType		getEventType(int eventIndex);
		bool									isSocketAcceptingNewConnection(int socketFd);

		void				createConnections();
		void				acceptNewConnection(int socketFd);


		void				nullifyDeletedConnectionReferences(
									Connection* connectionToDelete, 
									std::map<int, Connection*>::iterator connectionsStartIt
								);
};

#endif //MANAGER_HPP
