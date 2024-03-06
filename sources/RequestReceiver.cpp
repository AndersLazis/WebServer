/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestReceiver.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmenke <cmenke@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 11:12:44 by aputiev           #+#    #+#             */
/*   Updated: 2024/03/05 16:00:40 by cmenke           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestReceiver.hpp"
#include "RequestParser.hpp"
#include "logger/LoggerIncludes.hpp"

RequestReceiver::RequestReceiver(int fd)
               : _fd(fd), state(R_WAITING),
							   _header_pos(0), headersOk(false),
								 max_body_size(-1), received(0),
								 log(Logger::instance(), "RequestReceiver")
{
	this->parser.setRequest(this->req);
}

RequestReceiver::RequestReceiver()
               : _fd(-1), state(R_WAITING),
							   _header_pos(0), headersOk(false),
								 max_body_size(-1), received(0),
								 log(Logger::instance(), "RequestReceiver")
{
	this->parser.setRequest(this->req);
}

RequestReceiver::~RequestReceiver()
{}

RequestReceiver::RequestReceiver(const RequestReceiver &other)
               : log(Logger::instance(), "RequestReceiver")
{
	*this = other;
}

RequestReceiver &RequestReceiver::operator=(const RequestReceiver &other)
{
	if (this != &other)
	{
		this->_fd = other._fd;
		this->state = other.state;
		this->body = other.body;
		this->tmp_file = other.tmp_file;
		this->_header_pos = other._header_pos;
		this->req = other.req;
		this->parser = other.parser;
		this->headersOk = other.headersOk;
		this->error_code = other.error_code;
		this->max_body_size = other.max_body_size;
		this->received = other.received;
		this->log = other.log;
	}
	return *this;
}

/* ================ * Setters *  ================ */

void RequestReceiver::setMaxBodySize(ssize_t maxBodySize)
{
	this->max_body_size = maxBodySize;
}

/* ================ * Getters *  ================ */

StrContainer RequestReceiver::getBody() const
{
	return this->body;
}

std::string RequestReceiver::getTempFile() const
{
	return this->tmp_file;
}

ReceiverState RequestReceiver::getState() const
{
	return this->state;
}

Request &RequestReceiver::getRequest()
{
	return this->req;
}

int RequestReceiver::getFd() const
{
	return this->_fd;
}


void RequestReceiver::consume()
{
	if (this->headersOk)
	{
		if (this->req.content_length)
			this->receive_body();
		else
			this->state = R_FINISHED;
	}
	else
	{
		this->receive_headers();
		if (this->headersOk)
			this->state = R_REQUEST;
	}
}

bool RequestReceiver::receive_body()
{
	ssize_t bytes_read = recv(this->_fd, this->req.buff, this->req.buff_size, 0);
	this->received += bytes_read;
	//this->log << LOG_INFO << "received: " << this->received;
	if (this->max_body_size >= 0 && this->received > this->max_body_size)
		return this->finish_request("413");
	if (bytes_read < 0)
		throw std::runtime_error("Error receiving request: " + std::string(strerror(errno)));
	this->req.body_start = 0;
	this->req.offset = bytes_read;
	if (!bytes_read)
		this->state = R_FINISHED;
	return true;
}

bool RequestReceiver::receive_headers()
{
	ssize_t		bytes_read;

	bytes_read = recv(
		this->_fd,
		this->req.buff + this->req.offset,
		this->req.buff_size - this->req.offset,
		0
	);
	if (bytes_read < 0)
	{
		this->state = R_ERROR;
		this->log << LOG_ERROR << "received -1 for " << req.getUrl().getFullPath() << ": " << strerror(errno);
		this->error_code = StringData("500");
		return false;
	}
	if (bytes_read == 0)
	{
		//this->log << LOG_INFO << "received 0 for: " << this->req.getUrl().getPath();
		this->state = R_FINISHED;
		if (this->req.getUrl().getPath().empty())
			this->state = R_CLOSED;
		return true;
	}
	this->req.offset += bytes_read;


	std::string status;
	status = this->parser.parseRequestChunk(this->req.buff + this->_header_pos, this->req.offset - this->_header_pos);
	// this->log << LOG_DEBUG
	// 					 << "Parsed " << this->parser.getAmountParsed() << " bytes\n"
	// 					 << "Content: " << this->req.buff + this->_header_pos << "\n";
	this->_header_pos += this->parser.getAmountParsed();

	if (status != "")
	{
		// this->log << LOG_DEBUG
		// 					<< "Parsing status -> " << status
		// 					<< "\n";
		return this->finish_request(status);
	}
	//parsing complete
	else if (this->parser.parsingState == RequestParser::STATE_END)
	{
		std::map<std::string, std::string>& headers = this->req.headers;

		//handling host
		std::map<std::string, std::string>::const_iterator hostIt;
		hostIt = headers.find("host");

		if (hostIt == headers.end() || hostIt->second.empty())
		{
			this->log << LOG_INFO
			<< RED << getMethodString(this->req.getMethod()) << RESET
			<< " " << req.getUrl().getDomain() << ":" << req.getUrl().getPort() << req.getUrl().getFullPath()
			<< " missing host header\n";
			return this->finish_request("400");
		}
		else
		{
			std::string host = hostIt->second;
			std::string::size_type colonPos = host.find(':');
			if (colonPos != std::string::npos)
			{
				this->req.setDomain(host.substr(0, colonPos));
				this->req.setPort(host.substr(colonPos + 1));
			}
			else
			{
				this->req.setDomain(host);
				this->req.setPort("80");
			}
		}

		
		//handling content-length and transfer-encoding
		std::map<std::string, std::string>::const_iterator transferEncodingIt;
		std::map<std::string, std::string>::iterator contentLengthIt;
		transferEncodingIt = headers.find("transfer-encoding");
		contentLengthIt = headers.find("content-length");

		if (contentLengthIt != headers.end()
				&& transferEncodingIt != headers.end())
		{
			this->req.headers.erase(contentLengthIt);
		}
		if (transferEncodingIt != headers.end())
		{
			if (transferEncodingIt->second != "chunked") //changed since we dont support chunked encoding
			{
					this->log << LOG_INFO
					<< RED << getMethodString(this->req.getMethod()) << RESET
					<< " " << req.getUrl().getDomain() << ":" << req.getUrl().getPort() << req.getUrl().getFullPath()
					<< " unsupported transfer-encoding: " << transferEncodingIt->second << "\n";
					return this->finish_request("400");
			}
			this->req.content_length = -1;
			this->log << LOG_DEBUG
								<< "Transfer-Encoding: " << transferEncodingIt->second << "\n";
		}
		else if (contentLengthIt != headers.end())
		{
			std::istringstream iss(contentLengthIt->second);
			if (!std::isdigit(iss.peek()) //peek returns -1 in case of empty string 
					|| !(iss >> this->req.content_length && iss.eof()))
			{
				this->log << LOG_INFO
				<< RED << getMethodString(this->req.getMethod()) << RESET
				<< " " << req.getUrl().getDomain() << ":" << req.getUrl().getPort() << req.getUrl().getFullPath()
				<< " wrong content-length value: " << contentLengthIt->second << "\n";
				return this->finish_request("400");
			}
		}
		else if (this->req.getMethod() == POST)
		{
			this->log << LOG_INFO
			<< RED << getMethodString(this->req.getMethod()) << RESET
			<< " " << req.getUrl().getDomain() << ":" << req.getUrl().getPort() << req.getUrl().getFullPath()
			<< " missing content-length header\n";
			return this->finish_request("411");
		}
		else
		{
			this->req.content_length = 0;
		}
		this->headersOk = true;
		this->req.body_start = this->_header_pos;
		this->received = this->req.offset - this->req.body_start;
		return true;
	}

	// Check if the request is too large -> meaning everything till body start
	if (this->req.offset == Request::buff_size && !this->headersOk)
	{
		this->log << LOG_INFO
			<< RED << getMethodString(this->req.getMethod()) << RESET
			<< " " << req.getUrl().getDomain() << ":" << req.getUrl().getPort() << req.getUrl().getFullPath()
			<< " entity too large";
		return this->finish_request("413");
	}
	return false;
}

bool RequestReceiver::finish_request(const std::string& code)
{
	this->headersOk = false;
	this->state = R_ERROR;
	this->error_code = StringData(code);
	return false;
}

IData &RequestReceiver::produceData()
{
	if (
		this->max_body_size >= 0
		&& (
			this->received > this->max_body_size ||
			this->req.content_length > this->max_body_size
		)
	)
	{
		this->log << LOG_INFO
			<< RED << getMethodString(this->req.getMethod()) << RESET
			<< " " << req.getUrl().getDomain() << ":" << req.getUrl().getPort() << req.getUrl().getFullPath()
			<< " maximum body size of " << this->max_body_size << " exceeded";
		this->finish_request("413");
		this->state = R_FINISHED;
		return this->error_code;
	}
	switch (this->state)
	{
		case R_ERROR:
			this->state = R_FINISHED;
			return this->error_code;
			break;
		case R_REQUEST:
			if (this->headersOk && this->req.content_length)
				this->state = R_BODY;
			else if (this->headersOk)
				this->state = R_FINISHED;
			return this->req;
			break;
		case R_BODY:
			return this->req;
			break;
		default:
			break;
	}
	return (this->error_code);
}
