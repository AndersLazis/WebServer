#include "ResponseSender.hpp"

#include <Utils.hpp>

ResponseSender::ResponseSender():
	httpVersion("HTTP/1.1"), statusCode("200"), reason("OK"),
	sent(0), file_stream(NULL), fd(-1), ready(false), _finished(false), cgi(false), plain_sent(false),
	log(Logger::instance(), "ResponseSender")
{
}

ResponseSender::ResponseSender(int fd):
	httpVersion("HTTP/1.1"), statusCode("200"), reason("OK"),
	sent(0), file_stream(NULL), fd(fd), ready(false), _finished(false), cgi(false), plain_sent(false),
	log(Logger::instance(), "ResponseSender")
{
}

ResponseSender::~ResponseSender()
{
	if (this->file_stream)
	{
		if (this->file_stream->is_open())
			this->file_stream->close();
		delete this->file_stream;
	}
}

ResponseSender::ResponseSender(const ResponseSender &other)
              : log(Logger::instance(), "ResponseSender")
{
	*this = other;
}

ResponseSender ResponseSender::operator=(const ResponseSender &other)
{
	if (this != &other)
	{
		this->statusCode = other.statusCode;
		this->httpVersion = other.httpVersion;
		this->reason = other.reason;
		this->headers = other.headers;
		this->body = other.body;
		this->sent = other.sent;
		this->fd = other.fd;
		this->error_pages = other.error_pages;
		this->file = other.file;
		this->file_stream = other.file_stream;
		this->ready = other.ready;
		this->_finished = other._finished;
		this->cgi = other.cgi;
		this->plain_sent = other.plain_sent;
		this->log = other.log;
	}
	return *this;
}



void ResponseSender::setBody(std::string body)
{
	this->body = body;
}

void ResponseSender::setStatusCode(std::string code)
{
	this->statusCode = code;
}

void ResponseSender::setReason(std::string reason)
{
	this->reason = reason;
}

void ResponseSender::setHeader(std::string key, std::string value)
{
	this->headers[key] = value;
}

void ResponseSender::setContentTypes(std::string filename)
{
	FileTypes	file_types;
	this->setHeader("content-type", file_types.getFileType(filename));
}

void ResponseSender::setFd(int fd)
{
	this->fd = fd;
}

void ResponseSender::setErrorPages(std::map<int, std::string> map)
{
	this->error_pages = map;
}

void ResponseSender::setFile(std::string file)
{
	this->file = file;
}



std::string ResponseSender::getBody() const
{
	return this->body;
}

int ResponseSender::getFd() const
{
	return this->fd;
}

// Private

void ResponseSender::_build()
{
	this->_plain = this->httpVersion + " "
		+ this->statusCode + " "
		+ this->reason + "\r\n";
	this->body_size = this->body.size();
	if (this->body_size)
	{
		this->headers["content-length"] = Utils::toString(this->body_size);
	}
	for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); it++)
	{
		this->_plain += (it->first + ": " + it->second + "\r\n");
	}
	this->_plain += "\r\n";
	if (this->body_size)
		this->_plain += this->body;
}

// Public

bool ResponseSender::_send()
{
	size_t	left;
	size_t	chunk_size;
	ssize_t	chunk;
	if (this->_plain.empty())
		this->_build();
	chunk_size = 8192;
	left = this->_plain.size() - this->sent;
	if (left < chunk_size)
		chunk_size = left;
	if (!this->_plain.empty() && this->sent < this->_plain.size())
	{
		chunk = send(this->fd, this->_plain.c_str() + this->sent, chunk_size, SEND_FLAGS);
		if (chunk < 0)
			return false;
		this->sent += chunk;
		left -= chunk;
		if (left < chunk_size)
			chunk_size = left;
		if (this->file.empty() && this->sent >= this->_plain.size())
			return true;
		return false;
	}
	if (!this->file.empty() && this->file_stream)
	{
		if (!this->file_stream->is_open())
		{
			this->log << LOG_ERROR << "File is not open: " << this->file;
			return true;
		}
		if (!this->file_stream->good())
		{
			this->log << LOG_ERROR << "File is not good: " << this->file;
			this->file_stream->close();
			return true;
		}
		std::streamsize size;
		this->file_stream->seekg(0, std::ios::end);
		size = this->file_stream->tellg();
		left = this->_plain.size() + size - sent;
		chunk_size = 8192;
		if (left < chunk_size)
				chunk_size = left;
		this->file_stream->seekg(this->sent - this->_plain.size(), std::ios::beg);
		char buffer[chunk_size];
		if (this->file_stream->is_open() && this->sent < this->_plain.size() + size && !this->file_stream->eof())
		{
			//this->log << LOG_INFO << "Sent: " << this->sent << ", left: " << left << " from " << this->file;
			this->file_stream->read(buffer, chunk_size);
			int res = send(this->fd, buffer, this->file_stream->gcount(), SEND_FLAGS);
			if (res < 0)
				return false;
			this->sent += res;
			left -= res;
			if (this->sent < this->_plain.size() + size && !this->file_stream->eof())
				return false;
		}
		this->file_stream->close();
		delete this->file_stream;
		this->file_stream = NULL;
	}
	return true;
}

bool ResponseSender::run()
{
	char	buffer[80];

	time_t timestamp = time(NULL);
	struct tm *timeinfo = std::localtime(&timestamp); 
	std::strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);
	this->setHeader("date", buffer);
	this->setHeader("server", "Webserv42");
	return this->_send();
}

void ResponseSender::build_file(const std::string& filename, bool custom_error)
{
	std::istringstream iss(filename);
	StrContainer trimmedFileName;

	std::getline(iss, trimmedFileName);
	trimmedFileName.trim();
	if (this->file_stream && this->file_stream->is_open())
		return ;
	this->file_stream = new std::ifstream(trimmedFileName.c_str(), std::ios::binary | std::ios::ate);
	if (!this->file_stream->is_open())
	{
		this->log << LOG_ERROR << "File is not open: " << trimmedFileName;
		if (custom_error)
			this->build_error(this->statusCode, custom_error);
		else
			this->build_error("403", custom_error);
		return ;
	}
	if (!this->file_stream->good())
	{
		this->log << LOG_ERROR << "File is not good: " << trimmedFileName;
		this->file_stream->close();
		if (custom_error)
			this->build_error(this->statusCode, custom_error);
		else
			this->build_error("500", custom_error);
		return ;
	}
	this->setHeader("content-length", Utils::toString(this->file_stream->tellg()));
	this->setHeader("connection", "keep-alive");
	this->setContentTypes(trimmedFileName);
	this->setFile(trimmedFileName);
}

//sending error and success pages
void ResponseSender::build_error(const std::string& status_code, bool custom_error)
{
	this->_plain = "";
	this->file = "";
	if (this->file_stream)
	{
		if (this->file_stream->is_open())
			this->file_stream->close();
		delete this->file_stream;
		this->file_stream = NULL;
	}
	this->setStatusCode(status_code);
	this->setReason(HttpCodes::getReasonPhrase(status_code));
	int statusInt = std::atoi(status_code.c_str());
	std::map<int , std::string>::iterator it = this->error_pages.find(statusInt);
	if (it != this->error_pages.end() && !custom_error)
		return (this->build_file(this->error_pages[statusInt], true));
	this->setContentTypes("error.html");
	this->body = "<!DOCTYPE html><html><head><title>"
							 + HttpCodes::getCompleteStatus(status_code)
							 + "</title></head><body bgcolor='black'><center><h1><font color='white'>"
							 + HttpCodes::getCompleteStatus(status_code)
							 + "</font></h1></center></body></html>";
}

void ResponseSender::build_dir_listing(const std::string& content)
{
	static std::string filePath = "static/directoryListing.html";
	this->setContentTypes(filePath);

	std::ifstream directoryListingIfstream(filePath.c_str(), std::ios::binary);

	if (!directoryListingIfstream.is_open()) {
		this->log << LOG_INFO << "Directory listing template not open "
											              << filePath << "\n";
		this->build_error("500");
		return;
	}

	this->body = std::string((std::istreambuf_iterator<char>(directoryListingIfstream)),
														std::istreambuf_iterator<char>());

	if (directoryListingIfstream.fail()) {
		this->log << LOG_ERROR << "Error reading from directory listing template: "
																		 << filePath << "\n";
		this->build_error("500");
		return;
	}

	this->body += content;
	this->body += "</ul></body></html>";
}

void ResponseSender::build_redirect(const std::string& redirect)
{
	std::string	status_code = redirect.substr(0, 3);
	std::string	location = redirect.substr(3);
	this->_plain = "";
	this->setStatusCode(status_code);
	this->setReason(HttpCodes::getReasonPhrase(status_code));
	this->setHeader("Location", location);
	this->setContentTypes("redirect.html");
	this->body = "<!DOCTYPE html><html><head><title>"
		+ HttpCodes::getCompleteStatus(status_code)
		+ "</title></head><body><h1>"
		+ HttpCodes::getCompleteStatus(status_code)
		+ "</h1></body></html>";
}

void ResponseSender::build_cgi_response(const std::string& response)
{
	StrContainer new_response(response);
	if (!new_response.starts_with("Status:"))
	{
		size_t pos = new_response.find("\r\n\r\n");
		size_t location = new_response.find("Location:");
		if (location != std::string::npos && location < pos)
			new_response = this->httpVersion + " 302 Found\r\n" + new_response;
		else
			new_response = this->httpVersion + " 200 OK\r\n" + new_response;
	}
	else
		new_response.replace(0, 7, this->httpVersion, 0, this->httpVersion.size());
	this->_plain = new_response;
}

// ISender impl

void ResponseSender::setData(IData &data)
{
	this->cgi = false;
	try
	{
		StringData &d = dynamic_cast<StringData&>(data);
		switch (d.getType())
		{
			case D_ERROR:
				this->build_error(d);
				this->ready = true;
				break;
			case D_FILEPATH:
				this->build_file(d);
				this->ready = true;
				break;
			case D_DIRLISTING:
				this->build_dir_listing(d);
				this->ready = true;
				break;
			case D_REDIR:
				this->build_redirect(d);
				this->ready = true;
				break;
			case D_CGI:
				this->build_cgi_response(d);
				this->cgi = true;
				this->ready = true;
				break;
			case D_FINISHED:
				this->_finished = true;
				break;
			case D_NOTHING:
				this->ready = false;
				break ;
			default:
				break;
		}
	}
	catch(const std::bad_cast& e)
	{
		this->log << LOG_ERROR << e.what();
	}
}

bool ResponseSender::readyToSend()
{
	return this->ready;
}

bool ResponseSender::finished()
{
	return this->_finished;
}

void ResponseSender::sendData()
{
	bool res = this->run();
	if (this->statusCode == "100")
		this->_finished = false;
	else
		this->_finished = res;
}
