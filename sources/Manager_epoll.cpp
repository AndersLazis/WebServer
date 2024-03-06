#include "Manager.hpp"

//public

void Manager::addSocketToEpoll(int socketFd)
{
	struct epoll_event	epollEvent;
	std::memset(&epollEvent, 0, sizeof(epollEvent));

	epollEvent.events =  EPOLLIN;
	epollEvent.data.fd = socketFd;
  if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, socketFd, &epollEvent))
		throw std::runtime_error("Error adding SocketToEpoll (EPOLLIN): "
		                          + std::string(strerror(errno)));
}

void Manager::setEpollReadWriteFlags(int socketFd)
{
	struct epoll_event	epollEvent;
	std::memset(&epollEvent, 0, sizeof(epollEvent));

	epollEvent.events =  EPOLLIN | EPOLLOUT;
	epollEvent.data.fd = socketFd;
  if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, socketFd, &epollEvent))
		throw std::runtime_error("Error modifying SocketInEpoll (EPOLLIN | EPOLLOUT): "
		                          + std::string(strerror(errno)));
}

void Manager::setEpollReadFlag(int socketFd)
{
	struct epoll_event	epollEvent;
	std::memset(&epollEvent, 0, sizeof(epollEvent));

	epollEvent.events =  EPOLLIN;
	epollEvent.data.fd = socketFd;
  if (epoll_ctl(this->epollFd, EPOLL_CTL_MOD, socketFd, &epollEvent))
		throw std::runtime_error("Error modifying SocketInEpoll (EPOLLIN): "
		                          + std::string(strerror(errno)));
}

void Manager::removeSocketFromEpoll(int socketFd)
{
	if (epoll_ctl(this->epollFd, EPOLL_CTL_DEL, socketFd, NULL))
		throw std::runtime_error("Error removing Socket from Epoll EPOLL_CTL_DEL: "
		                          + std::string(strerror(errno)));
}

//private

void Manager::initEpoll()
{
	this->epollFd = epoll_create(1);
	if (this->epollFd < 0)
		throw std::runtime_error("Error creating epoll: " + std::string(strerror(errno)));
}

void Manager::addServerSocketToEpoll(int socketFd)
{
	struct epoll_event	epollEvent;
	std::memset(&epollEvent, 0, sizeof(epollEvent));

	epollEvent.events =  EPOLLIN | EPOLLET;
	epollEvent.data.fd = socketFd;
  if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, socketFd, &epollEvent))
		throw std::runtime_error("Error adding ServerSocketToEpoll (EPOLLIN | EPOLLET): "
		                          + std::string(strerror(errno)));
}

int Manager::getNewEpollEventsCount()
{
	return epoll_wait(this->epollFd, this->epollEvents,
	                  Manager::maxEpollEvents, -1);
}

int Manager::getEventFd(int eventIndex)
{
	return this->epollEvents[eventIndex].data.fd;
}

Manager::EventType Manager::getEventType(int eventIndex)
{
	if ((epollEvents[eventIndex].events & EPOLLERR)
		|| (epollEvents[eventIndex].events & EPOLLHUP))
		return Manager::EOF_CONN;
	if (this->isSocketAcceptingNewConnection(this->getEventFd(eventIndex)))
		return Manager::NEW_CONN;
	if ((epollEvents[eventIndex].events & EPOLLOUT) && (epollEvents[eventIndex].events & EPOLLIN))
		return Manager::READWRITE_AVAIL;
	if (epollEvents[eventIndex].events & EPOLLIN)
		return Manager::READ_AVAIL;
	if (epollEvents[eventIndex].events & EPOLLOUT)
		return Manager::WRITE_AVAIL;
	throw std::runtime_error("Unknown event!");
}

bool Manager::isSocketAcceptingNewConnection(int socketFd)
{
	return this->connections[socketFd]->getSocket() == socketFd;
}
