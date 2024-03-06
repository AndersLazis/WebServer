#include "URL.hpp"
#include "logger/LoggerIncludes.hpp"

URL::URL() {}

URL::URL(const URL &other) 
{
	*this = other;
}

URL::~URL() {}

URL &URL::operator=(const URL &other)
{
	if (this != &other)
	{
		this->domain = other.domain;
		this->port = other.port;
		this->path = other.path;
		this->query = other.query;
	}
	return *this;
}

URL URL::operator+(const URL &other)
{
	URL new_url = *this;
	new_url.path = URL::concatPaths(new_url.path, other.path);
	return new_url;
}

bool URL::operator==(const URL &other)
{
	if (this->domain.compare(other.domain))
		return false;
	if (this->port.compare(other.port))
		return false;
	if (this->path.compare(other.path))
		return false;
	if (this->query.compare(other.query))
		return false;
	return true;
}

void    URL::setDomain(StrContainer domain)
{
	domain.trim();
	this->domain = domain;
}

void    URL::setPort(StrContainer port)
{
	port.trim();
	this->port = port;
}

void		URL::setPath(StrContainer path)
{
	path.trim();
	this->path = path;
}

void    URL::setQuery(StrContainer query)
{
	query.trim();
	this->query = query;
}

StrContainer	URL::getDomain() const {
	return (this->domain);
}

StrContainer	URL::getPort() const {
	return (this->port);
}

StrContainer	URL::getPath() const {
	return (this->path);
}

StrContainer	URL::getQuery() const {
	return (this->query);
}

StrContainer URL::getFullPath() const
{
	StrContainer full_path;
	full_path += this->path;
	if (!this->query.empty())
		full_path += "?" + this->query;
	return (full_path);
}

// Public

StrContainer URL::concatPaths(StrContainer first, StrContainer second)
{
	if (first.ends_with("/"))
	{
		if (second.starts_with("/"))
		{
			second.erase(0);
			return first + second;
		}
		return first + second;
	}
	else
	{
		if (second.starts_with("/"))
			return first + second;
		return first + "/" + second;
	}
}

StrContainer URL::removeFromStart(StrContainer first, StrContainer second)
{
	if (first.starts_with(second))
		first.erase(0, second.size());
	return first;
}

StrContainer URL::removeFromEnd(StrContainer first, StrContainer second)
{
	if (first.ends_with(second))
		first.erase(first.size() - second.length() - 1);
	return first;
}

void URL::addSegment(StrContainer segment)
{
	this->path = URL::concatPaths(this->path, segment);
}