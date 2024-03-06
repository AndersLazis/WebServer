#include "handlers/StaticHandler.hpp"

StaticHandler::StaticHandler(): state(SH_START), log(Logger::instance(), "StaticHandler")
{
}

StaticHandler::StaticHandler(
	std::string	path,
	std::string	root_directory,
	bool		dir_listing,
	std::string	index,
	std::string	static_dir
): state(SH_START), log(Logger::instance(), "StaticHandler")
{
	this->path = path;
	this->root_directory = root_directory;
	this->dir_listing = dir_listing;
	this->index = index;
	this->static_dir = static_dir;
}

StaticHandler::~StaticHandler()
{}

StaticHandler::StaticHandler(const StaticHandler &other)
             : state(SH_START),
						   log(Logger::instance(), "StaticHandler")
{
	*this = other;
}

StaticHandler &StaticHandler::operator=(StaticHandler const &other)
{
	this->path = other.path;
	this->root_directory = other.root_directory;
	this->dir_listing = other.dir_listing;
	this->index = other.index;
	this->static_dir = other.static_dir;
	this->data = other.data;
	this->state = other.state;
	this->full_path = other.full_path;
	this->log = other.log;
	this->created = other.created;
	return *this;
}

// Private

static std::string errnoToHttpCodeDirectoryListing(int errnum)
{
	switch(errnum)
	{
		case EACCES:
			return "403";
		case EBADF:
		// fallthrough
		case EMFILE:
		// fallthrough
		case ENFILE:
		// fallthrough
		case ENOMEM:
			return "500";
		case ENOENT:
		// fallthrough
		case ENOTDIR:
			return "404";
		default:
			return "500";
	}
}

StringData StaticHandler::handle_dir_listing(Request req, std::string full_path)
{
	DIR *dir;
	struct dirent *ent;
	StrContainer content;
	StrContainer dir_content;

	StrContainer url_path = req.getUrl().getPath();
	errno = 0;
	dir = opendir(full_path.c_str());
	if (dir == NULL)
		return this->state = SH_FINISHED, StringData(errnoToHttpCodeDirectoryListing(errno));
	StrContainer path = url_path.substr(this->path.size(), url_path.size());
	if (path.empty() || !path.starts_with("/"))
		path = "/" + path;
	StrContainer route_path(this->path);
	if (!route_path.ends_with("/"))
		route_path += "/";
	while (1)
	{
		errno = 0;
		ent = readdir(dir);
		if (ent == NULL && errno == EBADF)
		{
			closedir(dir);
			return this->state = SH_FINISHED, StringData("500");
		}
		else if (ent == NULL)
		{
			break;
		}
		else if (strcmp(ent->d_name, ".") != 0)
		{
			std::string name = std::string(ent->d_name);
			content = "<a href=\"" + path + name;
			if (ent->d_type == DT_DIR)
			{
				content += "/\"><li>" + name + "/</li></a>";
			}
			else
			{
				content += "\"><li>" + name + "</li></a>";
			}
			dir_content += content;
			dir_content += "\n";
		}
	}
	closedir(dir);
	return this->state = SH_FINISHED, StringData(dir_content, D_DIRLISTING);
}

std::string StaticHandler::build_absolute_path(StrContainer requestPath)
{
	StrContainer	root(this->root_directory);
	StrContainer	req_path(requestPath);

	if (root.ends_with("/"))
		root.erase(root.size() - 1);
	if (!root.size())
		root = "html";
	if (requestPath.starts_with(this->path) && this->path != "/")
		req_path.erase(0, this->path.size());
	return root + req_path;
}

StringData StaticHandler::handle_delete(std::string full_path)
{
	if (std::remove(full_path.c_str()) == 0)
		return this->state = SH_FINISHED, StringData("200");
	return this->state = SH_FINISHED, StringData("403");
}

StringData StaticHandler::handle_create(Request &req, std::string full_path)
{
	std::ofstream output;
	output.open(full_path.c_str(), std::ios::out | std::ios::binary);
	this->full_path = full_path;
	if (!output.is_open() || !output.good())
		return this->state = SH_FINISHED, StringData("507");
	this->created = true;
	this->state = SH_UPLOADING;
	output.close();
	StringData res = req.save_chunk(full_path);
	if (res.getType() == D_FINISHED)
	{
		this->state = SH_FINISHED;
		return StringData("201");
	}
	else if (res.getType() == D_ERROR)
		this->state = SH_FINISHED;
	return res;
}

StringData StaticHandler::findFilePath(Request &req)
{
	if (!this->static_dir.empty()
		  && (req.getMethod() == POST || req.getMethod() == DELETE)
     )
	{
		this->root_directory = this->static_dir;
	}
	std::string full_path = this->build_absolute_path(req.getUrl().getPath());
	struct stat st;
	if (stat(full_path.c_str(), &st) == 0)
	{
		if (S_ISDIR(st.st_mode))
		{
			if (req.getMethod() == DELETE || req.getMethod() == POST)
			{
				return this->state = SH_FINISHED, StringData("403");
			}
			else if (full_path[full_path.size() - 1] == '/')
			{
				if (this->dir_listing) 
					return this->handle_dir_listing(req, full_path);
				full_path += this->index;
			}
			else
			{
				URL url = req.getUrl();
				url.addSegment("/");
				return this->state = SH_FINISHED, StringData("302" + url.getFullPath(), D_REDIR);
			}
		}
		else if (!(S_ISREG(st.st_mode)))
			return this->state = SH_FINISHED, StringData("403");
		if (req.getMethod() == GET)
			return this->state = SH_FINISHED, StringData(full_path, D_FILEPATH);
		else if (req.getMethod() == DELETE)
			return this->handle_delete(full_path);
		else if (req.getMethod() == POST) //exists already
			return this->state = SH_FINISHED, StringData("409");
	}
	else if (errno == EACCES)
	{
		return this->state = SH_FINISHED, StringData("403");
	}
	else if (req.getMethod() == POST
	         && full_path.find('/') != std::string::npos)
	{
		if (stat(full_path.substr(0, full_path.find_last_of('/')).c_str(), &st) == 0)
			return this->handle_create(req, full_path);
		else if (errno == EACCES)
			return this->state = SH_FINISHED, StringData("403");
	}
	return this->state = SH_FINISHED, StringData("404");
}

// IHandler impl

IData &StaticHandler::produceData()
{
	return this->data;
}

void StaticHandler::acceptData(IData &data)
{
	try
	{
		Request	&req = dynamic_cast<Request&>(data);
		//this->log << LOG_INFO << "accepting: " << req.getUrl().getFullPath() << " state: " << this->state << ", req chunked_state: " << req.getChunkedState();
		if (this->state == SH_START)
			this->data = this->findFilePath(req);
		else if (this->state == SH_UPLOADING)
		{
			this->data = req.save_chunk(this->full_path);
			if (this->data.getType() == D_FINISHED || this->data.getType() == D_ERROR)
			{
				this->state = SH_FINISHED;
				if (this->created)
					this->data = StringData("201");
				else
					this->data = StringData("200");
			}
		}
	}
	catch(const std::exception& e)
	{
		try {
			StringData &str = dynamic_cast<StringData&>(data);
			if (str.getType() == D_ERROR)
				this->data = str;
		}
		catch(const std::exception& e) {
			this->log << LOG_ERROR << "error accepting data: " << e.what();
		}
	}
}
