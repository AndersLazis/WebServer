#ifndef CONNECTION_HPP
# define CONNECTION_HPP
# include "Server.hpp"
# include "Address.hpp"
# include "Channel.hpp"
# include "handlers/ErrorHandler.hpp"
# include "handlers/CGIHandler.hpp"
# include "CGISender.hpp"
# include "CGIReceiver.hpp"

#include "logger/LoggerIncludes.hpp"

class Manager;

class Connection
{
	private:
		int								sock;
		std::map<std::string, Server>	servers; // Servers: key - server name
		std::map<int, Channel*>			channels; // key - socket for channel
		Address							address;
		Logger							log;
		Manager							*manager;
	public:
		Connection();
		~Connection();
		Connection(Address &addr);
		Connection(const Connection &other);
		Connection 						&operator=(const Connection &other);
		
		void							setmanager(Manager *manager);
		
		int								getSocket() const;
		Address							getAddress() const;
		// Public
		void							addServer(Server server);
		void							receive(int fd);
		void							send(int fd);
		bool							isCGI(int socket);
		void							deleteChannel(int fd);
};
#endif
