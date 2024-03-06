#include "Connection.hpp"
#include "Manager.hpp"

Connection::Connection()
				  : sock(-1), servers(),
						log(Logger::instance(), "Connection"),
					  manager(NULL)
{
}

Connection::~Connection()
{
	for (std::map<int, Channel *>::iterator it = this->channels.begin(); it != this->channels.end(); it++)
		delete it->second;
}

Connection::Connection(const Connection &other)
          :	sock(-1), servers(),
						log(Logger::instance(), "Connection"),
					  manager(NULL)
{
	*this = other;
}

Connection::Connection(Address &addr)
          :	sock(-1), servers(), address(addr),
						log(Logger::instance(), "Connection"),
					  manager(NULL)
{
	this->sock = socket(
		addr.getAddr()->ai_family,
		addr.getAddr()->ai_socktype,
		addr.getAddr()->ai_protocol
	);
	if (sock < 0)
	{
		this->~Connection();
		throw std::runtime_error(
			"Error creating connection socket: "
			+ std::string(strerror(errno))
		);
	}
	int	reuseaddr = 1;
	fcntl(sock, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &reuseaddr, sizeof(reuseaddr));
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
    if (bind(sock,
		addr.getAddr()->ai_addr,
		addr.getAddr()->ai_addrlen))
	{
		this->~Connection();
		close(sock);
		throw std::runtime_error(
			"Error binding: "
			+ std::string(strerror(errno))
		);
	}
    if (listen(sock, SOMAXCONN))
	{
		this->~Connection();
		close(sock);
		throw std::runtime_error(
			"Error listening: "
			+ std::string(strerror(errno))
		);
	}
	this->log << "\n";
	this->log << LOG_INFO << "listening " << CYAN << addr.getHost() << RESET;
}

Connection &Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		this->servers = other.servers;
		this->sock = other.sock;//dup(other.sock);
		this->log = other.log;
		this->address = other.address;
		this->manager = other.manager;
		this->channels = other.channels;
	}
	return *this;
}


void Connection::setmanager(Manager *manager)
{
	this->manager = manager;
}



int Connection::getSocket() const
{
		return this->sock;
}

Address Connection::getAddress() const
{
	return this->address;
}

// Public

void Connection::addServer(Server server)
{
	bool added = false;
	std::vector<std::string>	names = server.getServerNames();
	if (!this->servers.size())
	{
		this->log << LOG_INFO
			<< "Server " << server.printHosts()
			<< " has been added as " << BLUE << "default"
			<< " to " << GREEN << this->address.getHost() << RESET;
		this->servers["default"] = server;
		server.printRoutes();
		added = true;
	}
	for (size_t i = 0; i < names.size(); i++)
	{
		if (this->servers.find(names[i]) == this->servers.end())
		{
			this->log << LOG_INFO
				<< "Server " << server.printHosts()
				<< " has been added as " << BLUE << names[i] 
				<< " to " << GREEN << this->address.getHost() << RESET;
			this->servers[names[i]] = server;
			server.printRoutes(names[i] + ":" + this->getAddress().getPort());
			added = true;
		}
		else
			this->log << LOG_WARN
				<< "server name "
				<< BLUE << names[i] << RESET
				<< " is not used for "
				<< server.printHosts();
	}
	if (!added)
		this->log << LOG_WARN << "server " << server.printHosts() << " not used!";
}

void Connection::receive(int fd)
{
	if (this->channels.find(fd) != this->channels.end())
	{
		this->channels[fd]->receive();
	}
	else
	{
		this->channels[fd] = new Channel();
		this->channels[fd]->setReceiver(new RequestReceiver(fd));
		this->channels[fd]->setSender(new ResponseSender(fd));
		dynamic_cast<ResponseSender *>(this->channels[fd]->getSender())->setErrorPages(this->servers["default"].getErrorPages());
		this->channels[fd]->setHandler(new ErrorHandler());
		this->channels[fd]->receive();
	}
	switch (this->channels[fd]->getReceiver()->getState())
	{
		case R_WAITING:
			//this->log << LOG_INFO << "receiving: R_WAITING";
			return ;
			break;
		case R_ERROR:
			//this->log << LOG_INFO << "receiving: R_ERROR";
			this->channels[fd]->getHandler()->acceptData(this->channels[fd]->getReceiver()->produceData());
			this->manager->setEpollReadWriteFlags(fd);
			return ;
			break;
		case R_REQUEST:
		{
			//this->log << LOG_INFO << "receiving: R_REQUEST";
			StringData error("");
			Request &req = dynamic_cast<RequestReceiver *>(this->channels[fd]->getReceiver())->getRequest();
			if (this->servers.find(req.getUrl().getDomain()) != this->servers.end())
			{
				this->channels[fd]->setHandler(this->servers[req.getUrl().getDomain()].route(req, error));
				dynamic_cast<ResponseSender *>(this->channels[fd]->getSender())->setErrorPages(this->servers[req.getUrl().getDomain()].getErrorPages());
				dynamic_cast<RequestReceiver *>(this->channels[fd]->getReceiver())->setMaxBodySize(this->servers[req.getUrl().getDomain()].getMaxBodySize());
			}
			else
			{
				this->channels[fd]->setHandler(this->servers["default"].route(req, error));
				dynamic_cast<RequestReceiver *>(this->channels[fd]->getReceiver())->setMaxBodySize(this->servers["default"].getMaxBodySize());
			}
			if (!error.empty())
				this->channels[fd]->getHandler()->acceptData(error);
			else
			{
				this->channels[fd]->getHandler()->acceptData(this->channels[fd]->getReceiver()->produceData());
				CGIHandler	*cgiHandler = dynamic_cast<CGIHandler *>(this->channels[fd]->getHandler());
				if (cgiHandler && cgiHandler->getFd() > 0)
				{
					this->channels[cgiHandler->getFd()] = new Channel();
					this->channels[cgiHandler->getFd()]->setReceiver(new CGIReceiver(cgiHandler->getFd()));
					this->channels[cgiHandler->getFd()]->setSender(new CGISender(cgiHandler->getFd()));
					this->channels[cgiHandler->getFd()]->setHandler(cgiHandler);
					this->manager->addConnection(cgiHandler->getFd(), this);
					this->manager->addSocketToEpoll(cgiHandler->getFd());
					if (req.content_length)
						this->manager->setEpollReadWriteFlags(cgiHandler->getFd());
				}
			}
			this->manager->setEpollReadWriteFlags(fd);
			break ;
		}
		case R_BODY:
		{
			//this->log << LOG_INFO << "receiving: R_BODY";
			this->channels[fd]->getHandler()->acceptData(this->channels[fd]->getReceiver()->produceData());
			CGIHandler	*cgiHandler = dynamic_cast<CGIHandler *>(this->channels[fd]->getHandler());
			RequestReceiver *rec = dynamic_cast<RequestReceiver *>(this->channels[fd]->getReceiver());
			if (rec && cgiHandler && cgiHandler->getFd() > 0)
			{
				this->channels[cgiHandler->getFd()] = new Channel();
				this->channels[cgiHandler->getFd()]->setReceiver(new CGIReceiver(cgiHandler->getFd()));
				this->channels[cgiHandler->getFd()]->setSender(new CGISender(cgiHandler->getFd()));
				this->channels[cgiHandler->getFd()]->setHandler(cgiHandler);
				this->manager->addConnection(cgiHandler->getFd(), this);
				this->manager->addSocketToEpoll(cgiHandler->getFd());
				if (rec->getRequest().content_length)
					this->manager->setEpollReadWriteFlags(cgiHandler->getFd());
			}
			break;
		}
		case R_FINISHED:
			break;
		case R_CLOSED:
		{
			delete this->channels[fd];
			this->channels.erase(fd);
			this->manager->removeSocketFromEpoll(fd);
			this->manager->removeConnection(fd);
			close(fd);
		}
		default:
			break;
	}
}

void Connection::send(int fd)
{
	if (this->channels.find(fd) != this->channels.end())
	{
		this->channels[fd]->getSender()->setData(this->channels[fd]->getHandler()->produceData());
		this->channels[fd]->send();
		if (this->channels[fd]->senderFinished())
		{
			if (dynamic_cast<CGISender *>(this->channels[fd]->getSender()))
			{
				CGIHandler *cgiHandler = dynamic_cast<CGIHandler *>(this->channels[fd]->getHandler());
				cgiHandler->removeTmpFile();
				this->manager->setEpollReadFlag(cgiHandler->getFd());
				return ;
			}
			this->deleteChannel(fd);
		}
	}
}

bool Connection::isCGI(int socket)
{
	if (this->channels.find(socket) == this->channels.end())
		return false;
	return (
		dynamic_cast<RequestReceiver *>(
			this->channels[socket]->getReceiver()
		) == NULL
	);
}

void Connection::deleteChannel(int fd)
{
	// this->manager->removeSocketFromEpoll(fd);
	close(fd);
	this->manager->removeConnection(fd);
	if (this->channels.find(fd) == this->channels.end())
		return ;
	if (dynamic_cast<RequestReceiver *>(this->channels[fd]->getReceiver()))
	{
		IHandler *handler = this->channels[fd]->getHandler();
		if (handler && dynamic_cast<CGIHandler *>(handler))
		{
			int cgiFd = dynamic_cast<CGIHandler *>(handler)->getFd();
			if (cgiFd >= 0)
				this->deleteChannel(cgiFd);
		}
	}
	delete this->channels[fd];
	this->channels.erase(fd);
}
