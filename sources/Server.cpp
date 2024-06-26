#include "Server.hpp"

Server::Server():
	routes(), hosts(), server_names(), error_pages(), max_body_size(-1),
	log(Logger::instance(), "Server")
{}

Server::~Server()
{}

Server::Server(const Server &other)
      : log(Logger::instance(), "Server"), 
			  env(NULL)
{
	*this = other;
}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		this->routes = other.routes;
		this->hosts = other.hosts;
		this->server_names = other.server_names;
		this->error_pages = other.error_pages;
		this->max_body_size = other.max_body_size;
		this->log = other.log;
		this->env = other.env;
	}
	return (*this);
}



void Server::setRoute(Route &route)
{
	this->routes.push_back(route);
}

void Server::setHosts(std::string host, std::string port)
{
	this->hosts.insert(std::pair<std::string, std::string>(host, port));
}

void Server::setServerNames(std::stringstream &ss)
{
	std::string word;
	while (ss >> word)
		this->server_names.insert(this->server_names.end(), word);
}

void Server::setEnv(char **env)
{
	this->env = env;
}

void Server::setMaxBodySize(long long bodySize)
{
	this->max_body_size = bodySize;
}

void Server::setErrorPage(int code, std::string path)
{
	this->error_pages[code] = path;
}



char **Server::getEnv() const
{
	return this->env;
}

long long Server::getMaxBodySize() const
{
	return this->max_body_size;
}

std::vector<std::string> Server::getServerNames() const
{
	return this->server_names;
}

std::multimap<std::string, std::string> Server::getHosts() const
{
	return this->hosts;
}

std::map<int, std::string> Server::getErrorPages() const
{
	return (this->error_pages);
}

// Private

Route &Server::select_route(const Request &req)
{
	size_t		max_size = 0;
	size_t		curr_size;
	std::vector<Route>::iterator	res;
	for (
		std::vector<Route>::iterator it = this->routes.begin();
		it != this->routes.end();
		it++
	)
	{
		curr_size = it->match(req.getUrl().getPath());
		if (curr_size > max_size)
		{
			max_size = curr_size;
			res = it;
		}
	}
	if (!max_size)
		throw std::runtime_error(req.getUrl().getFullPath() + ": no matching route!");
	this->log << LOG_INFO
		<< CYAN << getMethodString(req.getMethod()) << RESET
		<< " " << req.getUrl().getDomain() << ":" << req.getUrl().getPort() << req.getUrl().getPath()
		<< "\t=> " BLUE << res->getPath() << " " << res->getFileExt() << RESET;
	return *res;
}

// Public

bool Server::hasListenDup() {
	std::set<std::pair<std::string, std::string> > s(this->hosts.begin(), this->hosts.end());
	return (s.size() != this->hosts.size());
}

void Server::printRoutes(std::string name)
{
	for (size_t i = 0; i < this->routes.size(); i++)
	{
		std::string route_type;
		RouteType r = this->routes[i].getType();
		switch (r)
		{
			case PATH_:
				route_type = "static:\t";
				break;
			case REDIRECTION_:
				route_type = "redir:\t";
				break;
			case CGI_:
				route_type = "cgi: \t";
				break;
			default:
				route_type = "unkn:\t";
				break;
		}
		if (name.empty())
			name = this->hosts.begin()->first + ":" + this->hosts.begin()->second;
		this->log << LOG_INFO
			<< route_type
			<< BLUE << "http://" << name << this->routes[i].getPath() << " " << this->routes[i].getFileExt() << RESET
			<< (this->routes[i].getRedirectUrl().size() ? "\t=> " + this->routes[i].getRedirectUrl() : "")
			<< (this->routes[i].getType() == CGI_ ? " " + this->routes[i].getCGIExt() : "");
	}
}

void		Server::printServer() {
	this->log << LOG_INFO << GREEN << "------------- * Server * ---------------" << RESET;
	for (std::multimap<std::string, std::string>::iterator i = this->hosts.begin(); i != this->hosts.end(); i++)
		this->log << LOG_INFO << "Host: " << i->first << " Port: " << i->second;
	this->log << LOG_INFO << "Client Max body size : " << this->max_body_size;
	for (long unsigned int i = 0; i < this->server_names.size(); i++)
		this->log << LOG_INFO << "Server name [" << i  << "]: " << this->server_names[i];
	for (std::map<int, std::string>::iterator i = this->error_pages.begin(); i != this->error_pages.end(); i++)
		this->log << LOG_INFO << "Error code path["<< i->first << "]: " << i->second;
	for (std::vector<Route>::iterator i = this->routes.begin(); i != this->routes.end(); i++)
	{		//this->log << LOG_INFO << "                *";
		this->log << LOG_INFO << BLUE << "Route path: " << i->getPath() << RESET;
		i->printRoute();
	}
}

std::string Server::printHosts()
{
	std::stringstream res;
	res << "\" ";
	for (
		std::multimap<std::string, std::string>::iterator it = this->hosts.begin();
		it != this->hosts.end();
		it++
	)
	{
		res << (it->first + ":" + it->second);
	}
	if (this->server_names.size())
	{
		res << " * ";
		for(
			std::vector<std::string>::iterator it = this->server_names.begin();
			it != this->server_names.end();
			it++
		)
		{
			res << *it;
			if (it + 1 != this->server_names.end())
				res << " ";
		}
	}
	res << " \"";
	return res.str();
}

// IRouter impl

IHandler *Server::route(IData &request, StringData &error)
{
	Request &req = dynamic_cast<Request&>(request);
	return this->select_route(req).route(request, error);
}


bool	Server::isLocationPathUnique(const std::string& path)
{
	for (std::vector<Route>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
	{
		if (it->getPath() == path)
			return (false);
	}
	return (true);
}


void Server::deleteroutes()
{
	this->routes.clear();
	
}