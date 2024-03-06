

#include "handlers/CGIHandler.hpp"

CGIHandler::CGIHandler():
	fd(-1),
	pid(-1),
	dataForResponse(StringData("", D_NOTHING)),
	tmp_file(""),
	log(Logger::instance(), "CGIHandler"),
	finished(false)
{
}

CGIHandler::CGIHandler(
	const std::string& 			path,
	const std::vector<Method>	&allowed_methods,
	const std::string&			root_directory,
	const std::string&			index,
	const std::string&			path_to_script,
	const CGI&					cgi
):
	fd(-1),
	pid(-1),
	dataForResponse(StringData("", D_NOTHING)),
	tmp_file(""),
	log(Logger::instance(), "CGIHandler"),
	finished(false)
{
	this->path = path;
	this->allowed_methods = allowed_methods;
	this->root_directory = root_directory;
	this->index = index;
	this->path_to_script = path_to_script;
	this->cgi = cgi;
}

CGIHandler::~CGIHandler()
{
	if (!this->tmp_file.empty())
		std::remove(this->tmp_file.c_str());
}

CGIHandler::CGIHandler(const CGIHandler &other)
          : fd(-1), pid(-1),
					  log(Logger::instance(), "CGIHandler")
{
	*this = other;
}

CGIHandler &CGIHandler::operator=(CGIHandler const &other)
{
	this->fd = other.fd;
	this->pid = other.pid;
	this->path = other.path;
	this->allowed_methods = other.allowed_methods;
	this->root_directory = other.root_directory;
	this->index = other.index;
	this->path_to_script = other.path_to_script;
	this->cgi = other.cgi;
	this->dataForResponse = other.dataForResponse;
	this->tmp_file = other.tmp_file;
	this->log = other.log;
	this->finished = other.finished;
	return *this;
}



int CGIHandler::getFd()
{
	return this->fd;
}

// Private

std::string CGIHandler::build_absolute_path(const StrContainer& requestPath)
{
	StrContainer	root(this->root_directory);
	StrContainer	req_path(requestPath);

	if (root.ends_with("/"))
		root.erase(root.size() - 1);
	if (root.empty())
		root = "html";
	if (requestPath.starts_with(this->path) && this->path != "/")
		req_path.erase(0, this->path.size());
	return root + req_path;
}

void CGIHandler::configureCGI(Request &req)
{
	pid_t tmp_pid;
	int sv[2];

	if (req.content_length == -1)
	{
		std::ifstream tmp(
			this->tmp_file.c_str(),
			std::ifstream::ate | std::ifstream::binary
		);
		if (!tmp.is_open() || !tmp.good())
		{
			this->dataForResponse = StringData("500");
			return ;
		}
		std::stringstream ss;
		ss << tmp.tellg();
		req.removeHeader("transfer-encoding");
		req.setHeader("content-length", ss.str());
		tmp.close();
	}
	this->cgi.createEnv(req);
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1)
	{
		this->dataForResponse = StringData("501");
		return ;
	}
	this->fd = sv[0];
	fcntl(this->fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
	if ((tmp_pid = fork()) == -1)
	{
		this->dataForResponse = StringData("502");
		return ;
	}
	if (tmp_pid == 0)
	{
		if (this->cgi.execute(req, sv, this->path_to_script))
			exit(EXIT_FAILURE);
	}
	else
	{
		close(sv[1]);
		this->pid = tmp_pid;
	}
}

void CGIHandler::removeTmpFile()
{
	//this->log << LOG_INFO << this << " removing " << this->tmp_file;
	this->dataForResponse = StringData("", D_NOTHING);
	if (!this->tmp_file.empty())
		std::remove(this->tmp_file.c_str());
	this->tmp_file.clear();
}

int CGIHandler::_rand()
{
    static int seed = time(NULL);
    seed = (seed * 1103515245 + 12345) % 2147483647;
    return seed;
}

// IHandler impl

IData &CGIHandler::produceData()
{
	int	status = 0;
	if (this->pid >= 0)
	{
		waitpid(this->pid, &status, WNOHANG);
		if (WIFEXITED(status) && WEXITSTATUS(status))
		{
			// this->log << LOG_INFO << "cgi finished with status code: " << WEXITSTATUS(status);
			this->dataForResponse = StringData("500");
		}
		else if (!this->tmp_file.empty())
			this->dataForResponse = StringData(this->tmp_file, D_TMPFILE);
	}
	return this->dataForResponse;
}

void CGIHandler::acceptData(IData &data)
{
	try
	{
		Request &req = dynamic_cast<Request &>(data);
		if (req.content_length && !req.isBodyReceived())
		{
			if (this->tmp_file.empty())
			{
				std::stringstream ss;
				ss << "cgi_tmp" << this->_rand();
				this->tmp_file = ss.str();
			}
			req.save_chunk(this->tmp_file);
		}
		if (req.isBodyReceived())
			this->configureCGI(req);
	}
	catch(const std::exception& e)
	{
		try
		{
			StringData rsp = dynamic_cast<StringData &>(data);
			this->dataForResponse = rsp;
		}
		catch(const std::exception& e)
		{
			this->log << LOG_ERROR << "accepting unknown data type";
		}
	}	
}

std::string CGIHandler::getRoot() {
	return this->root_directory;
}

std::string CGIHandler::getIndex() {
	return this->index;
}

std::string CGIHandler::getPath() {
	return this->path;
}
