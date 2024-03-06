#include "Manager.hpp"
#include "Address.hpp"
#include "Utils.hpp"

// Public

bool Manager::running = true;

void Manager::signalHandlerSigint(int sigNum)
{
	if (sigNum == SIGINT)
		Manager::running = false;
}

Manager::Manager(std::vector<Server> servers, char **env)
	  : log(Logger::instance(), "Manager"),
		  epollFd(-1), ev(env)
{	
	this->servers = servers;
}

Manager::~Manager()
{
	typedef std::map<int, Connection*>::iterator MapIterator;
	
	MapIterator connectionsIt = this->connections.begin();
	for (; connectionsIt != this->connections.end(); ++connectionsIt)
	{
		if (connectionsIt->first >= 0)
		{
			close(connectionsIt->first);
		}
		if (connectionsIt->second)
		{
			Connection* connectionToDelete;
			connectionToDelete = connectionsIt->second;

			this->nullifyDeletedConnectionReferences(connectionToDelete, connectionsIt);
			delete connectionToDelete;
		}
	}
	if (this->epollFd >= 0)
	{
		close(this->epollFd);
	}
}

void Manager::startServer()
{
	this->initEpoll();
	this->createConnections();

	std::map<int, Connection*>::iterator connectionsIt;

	for (
		connectionsIt = this->connections.begin();
		connectionsIt != this->connections.end();
		++connectionsIt
	)
	{
		if (connectionsIt->first == connectionsIt->second->getSocket())
			this->addServerSocketToEpoll(connectionsIt->first);
	}
	signal(SIGINT, &Manager::signalHandlerSigint);
	this->run();
}

void Manager::addConnection(int fd, Connection *conn)
{
	this->connections[fd] = conn;
}

void Manager::removeConnection(int socket)
{
	this->connections.erase(socket);
}

// Private

Manager& Manager::operator=(const Manager &other)
{
	(void)other;
	return *this;
}

void Manager::run()
{
	int	num_events;
	int	event_sock;
	
	while (Manager::running)
	{	
		try
		{
			num_events = this->getNewEpollEventsCount();
			if (num_events < 0)
			{
				if (errno != EINTR)
					throw std::runtime_error("Queue error: " + std::string(strerror(errno)));
			}
			for (int i = 0; i < num_events; i++)
			{
				event_sock = this->getEventFd(i);
				if (this->connections.find(event_sock) == this->connections.end())
				{
					//this->log << LOG_INFO << "Trash socket: " << event_sock;
					//this->removeSocketFromEpoll(event_sock);
					//this->log << LOG_INFO << close(event_sock);
					continue ;
					
				}
				
				switch (this->getEventType(i))
				{
					case Manager::NEW_CONN:
						// this->log << LOG_INFO << "NEW conn " << event_sock;
						this->acceptNewConnection(event_sock);
						break;
					case Manager::EOF_CONN:
						//this->log << LOG_INFO << "EOF conn " << event_sock;
						if (this->connections.find(event_sock) != this->connections.end() && this->connections[event_sock] && this->connections[event_sock]->isCGI(event_sock))
						{
							//this->log << LOG_INFO << "IS CGI: " << event_sock;
							this->connections[event_sock]->receive(event_sock);
						}
						else
						{
							//this->removeSocketFromEpoll(event_sock);
							this->connections[event_sock]->deleteChannel(event_sock);
							//this->connections.erase(event_sock);
							//close(event_sock);
						}
						break;
					case Manager::READ_AVAIL:
						//this->log << LOG_INFO << "Read available " << event_sock;
						this->connections[event_sock]->receive(event_sock);
						break;
					case Manager::WRITE_AVAIL:
						//this->log << LOG_INFO << "Write available " << event_sock;
						if (this->connections.find(event_sock) != this->connections.end())
							this->connections[event_sock]->send(event_sock);
						break;
					case Manager::READWRITE_AVAIL:
						//this->log << LOG_INFO << "ReadWrite available " << event_sock;
						this->connections[event_sock]->receive(event_sock);
						if (this->connections.find(event_sock) != this->connections.end())
							this->connections[event_sock]->send(event_sock);
						break;
					default:
						this->log << LOG_INFO << "Unknown event type " << event_sock;
						break;
				}
			}
		}
		catch(const std::exception& e)
		{
			this->log << LOG_ERROR << e.what();
		}
	}
}

void Manager::createConnections()
{
	for (size_t i = 0; i < this->servers.size(); i++)
	{
		std::multimap<std::string, std::string> hosts = this->servers[i].getHosts();
		for (std::multimap<std::string, std::string>::iterator it = hosts.begin();	it != hosts.end();	it++)
		{
			Address address(it->first, it->second);
			bool already_exists = false;
			for (std::map<int, Connection*>::iterator it = this->connections.begin(); it != this->connections.end();	it++)
			{
				if (it->second->getAddress() == address)
				{
					it->second->addServer(this->servers[i]);
					already_exists = true;
					break ;
				}
			}
			if (!already_exists)
			{
				Connection *conn = new Connection(address);
				conn->addServer(this->servers[i]);
				conn->setmanager(this);
				this->connections[conn->getSocket()] = conn;
			}
		}
	}
}

void Manager::acceptNewConnection(int socketFd)
{
	int					newConnectionFd;
	static int	retryCount = 0;

	// Get local listeningSocket address
  struct sockaddr_in listeningAddr;
  socklen_t listeningAddrLen = sizeof(listeningAddr);
	std::string listeningAddress = "";
	std::string listeningPort = "";
	if (getsockname(socketFd, (struct sockaddr *)&listeningAddr, &listeningAddrLen) != -1)
	{
		uint32_t listeningIpv4 = ntohl(listeningAddr.sin_addr.s_addr);
		listeningAddress = Address::ipv4ToString(listeningIpv4);
		listeningPort = Utils::toString(ntohs(listeningAddr.sin_port));
	}

	while (retryCount < Manager::maxNetworkErrorRetries)
  {
    struct sockaddr_in remoteAddr; // sockaddr_in for IPv4 addresses
    socklen_t remoteAddrLen;
    remoteAddrLen = sizeof(remoteAddr);

    newConnectionFd = accept(socketFd, (struct sockaddr *)&remoteAddr, &remoteAddrLen);
    if (newConnectionFd == -1)
    {
			// blocking try again in next epoll event
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// this->log << LOG_DEBUG << "accept failed on socketFd: " << socketFd << "\n"
				// 										   << "port: " << listeningPort << " address: " << listeningAddress << "\n"
				// 										   << strerror(errno) << "\n"
				// 										   << "trying again...\n";
				break;
			}
			// Network errors in TCP
			// try again in next epoll event
			else if (retryCount < Manager::maxNetworkErrorRetries
							&& (errno == ENETDOWN || errno == EPROTO || errno == ENOPROTOOPT || errno == EHOSTDOWN ||
							    errno == ENONET || errno == EHOSTUNREACH || errno == EOPNOTSUPP || errno == ENETUNREACH))
			{
				// this->log << LOG_DEBUG << "accept failed on socketFd: " << socketFd  << "\n"
				// 															 << "port: " << listeningPort << " address: " << listeningAddress  << "\n"
				// 															 << strerror(errno)  << "\n"
				// 															 << "retrying... ("
				// 															 << retryCount << "/" << Manager::maxNetworkErrorRetries
				// 															 << ")\n";
      	retryCount++;
				break;
      }
			// Other fatal errors
      else
      {
				std::string logMessage = "accept failed on socketFd: " + Utils::toString(socketFd) + "\n"
											           + "port: " + listeningPort + " address: " + listeningAddress + "\n"
											           + strerror(errno) + "\n";
        throw std::runtime_error(logMessage);
      }
    }

		// Get remote socket address
		uint32_t remoteIpv4 = ntohl(remoteAddr.sin_addr.s_addr);
		std::string remoteAddress = Address::ipv4ToString(remoteIpv4);
		std::string remotePort = Utils::toString(ntohs(remoteAddr.sin_port));

		// setting socket flags
		int enable = 1;
		int socketFlags = SO_RCVBUF | SO_SNDBUF;
		// setting the socket max buffer for send and receive
		if (setsockopt(newConnectionFd, SOL_SOCKET, socketFlags, &enable, 8192) == -1
		    || fcntl(newConnectionFd, F_SETFL, O_NONBLOCK | FD_CLOEXEC) == -1)
		{
			close(newConnectionFd);
			std::string logMessage =  "setting options on socket failed on newConnectionFd: "
			                          + Utils::toString(newConnectionFd) + "\n"
																+ "listeningAddress: " + listeningAddress + ", listeningPort: " + listeningPort + "\n"
											          + "remoteAddress: " + remoteAddress + ", remotePort: " + remotePort  + "\n"
											          + strerror(errno) + "\n";
			throw std::runtime_error(logMessage);
		}

	  // The client connection and the original listening socket share the same Connection object
		this->addSocketToEpoll(newConnectionFd);
		this->connections[newConnectionFd] = this->connections[socketFd];
		retryCount = 0;

		// this->log << LOG_DEBUG << "Accepted new connection: "  << "\n"
		// 									     << "listeningAddress: " << listeningAddress << ", listeningPort: " << listeningPort << "\n"
		// 									     << "remoteAddress: " << remoteAddress << ", remotePort: " << remotePort  << "\n";
  }
}

void Manager::nullifyDeletedConnectionReferences(
	Connection* connectionToDelete, 
	std::map<int, Connection*>::iterator connectionsStartIt
)
{
	std::map<int, Connection*>::iterator connectionsIt;

	connectionsIt = connectionsStartIt;
	for (; connectionsIt != this->connections.end(); ++connectionsIt)
	{
		if (connectionsIt->second == connectionToDelete)
		{
			connectionsIt->second = NULL;
		}
	}
}
