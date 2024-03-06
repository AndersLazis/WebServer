/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmenke <cmenke@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 12:24:01 by aputiev           #+#    #+#             */
/*   Updated: 2024/03/05 16:48:47 by cmenke           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "CGI.hpp"
#include <cstdio>
#include <set>
#include <sstream>
#include <iostream>

CGI::CGI()
   : handler(), env(NULL), executablePath(),
	   enabled(false),
		 log(Logger::instance(), "CGI")
{
}

CGI::CGI(const std::vector<std::string>& handler, char **env)
   : handler(handler), env(NULL),
	   enabled(true),
		 log(Logger::instance(), "CGI")
{
	char *path = ft_getEnv(env);
	this->setPaths(path);
	this->setExecutablePath();
}

char* CGI::ft_getEnv(char **env) const
{
	int i = -1;
	if (env == NULL)
		return (NULL);
	while (env[++i])
	{
		if (!strncmp(env[i], "PATH=", 5))
			return (env[i] + 5);
	}
	return (NULL);
}

CGI::~CGI()
{
	if (this->env)
	{
		for (int i = 0; this->env[i]; i++)
		{
			delete[] this->env[i];
		}
		delete[] this->env;
	}
	
}

CGI::CGI(const CGI &other)
   : handler(), env(NULL), executablePath(),
	   enabled(false),
	   log(Logger::instance(), "CGI")
{
	*this = other;
}

CGI	&CGI::operator=(const CGI &other)
{
	if (this != &other)
	{
		this->cgiExt = other.cgiExt;
		this->handler = other.handler;
		this->executablePath = other.executablePath;
		this->env = other.env;
		this->paths = other.paths;
		this->scriptName = other.scriptName;
		this->scriptFilename = other.scriptFilename;
		this->pathInfo = other.pathInfo;
		this->pathTranslated = other.pathTranslated;
		this->documentRoot = other.documentRoot;
		this->prevURL = other.prevURL;
		this->prevExecPath = other.prevExecPath;
		this->enabled = other.enabled;
		this->log = other.log;
		this->requestURI = other.requestURI;
	}
	return (*this);
}


std::vector<std::string>	CGI::getHandler()
{
	return (this->handler);
}

char **CGI::getEnv()
{
	return this->env;
}

std::string CGI::getCgiExt() const {
        return (this->cgiExt);
}

URL CGI::getPrevURL() const {
	return (this->prevURL);
}

std::string CGI::getExecutablePath(const std::string& full_path)
{
	if (this->executablePath.empty())
		return full_path;
	return this->executablePath;
}

char **CGI::getArgs(const std::string& full_path)
{
	if (this->handler.size() != 1)
		throw std::runtime_error("CGI::getArgs: handler size is not 1");
	char **args = new char*[3];
	args[0] = NULL;
	args[1] = NULL;
	args[2] = NULL;

	try
	{
		args[0] = new char[this->handler[0].size() + 1];
		args[1] = new char[full_path.size() + 1];
	}
	catch (const std::bad_alloc& e)
	{
		delete[] args[0];
		delete[] args[1];
		delete[] args;
		throw std::runtime_error("CGI::getArgs: " + std::string(e.what()));
	}
	args[0] = strcpy(args[0], this->handler[0].c_str());
	args[1] = strcpy(args[1], full_path.c_str());
	return args;
}

StrContainer CGI::getPrevExecPath() const {
	return (this->prevExecPath);
}


void CGI::setEnv(char ** envp)
{
	this->env = envp;
}

void CGI::setHandler(const std::vector<std::string>& original)
{
	this->handler = original;
}

void	CGI::setPaths(const char *path)
{
	if (!path)
		return ;
	std::stringstream ss(path);
	std::string token;
	while (std::getline(ss, token, ':'))
	{
		if (token[token.length() - 1] == '/')
			token.erase(token.length() - 1);
		this->paths.push_back(token);
	}
}

void CGI::setExecutablePath()
{
	if (this->handler[0] == "$self")
		return;
	if (this->handler[0][0] != '/')
	{
		this->executablePath = this->findExecutablePath(this->paths, this->handler[0]);
                if (!this->executablePath.empty())
		        this->handler[0] = this->executablePath;
	}
	else
		this->executablePath = handler[0];
}

std::string CGI::findExecutablePath(std::vector<std::string> paths,
                                    std::string handler) const
{
	for (std::vector<std::string>::iterator it = paths.begin(); it != paths.end(); it++)
	{
		std::string path = *it + "/" + handler;
		if (access(path.c_str(), X_OK) == 0)
			return (path);
	}
	return ("");
}

void    CGI::setCgiExt(const std::string& ext)
{
        this->cgiExt = ext;
}

// Public
int CGI::execute(Request &req, int *sv, std::string& full_path) {
	(void) req;
	close(sv[0]);
	dup2(sv[1], 0);
	dup2(sv[1], 1);
	close(sv[1]);
	int pos = full_path.find_last_of('/');
	std::string dir = full_path.substr(0, pos);
	if (!full_path.empty() && full_path[0] != '/')
	{
		full_path = full_path.substr(pos + 1);
	}
	if (chdir(dir.c_str()) == -1)
		return (1);
	char **args = this->getArgs(full_path);
	execve(args[0], args, this->getEnv());
	delete[] args[0];
	delete[] args[1];
	delete[] args;
	return (1);
}

StrContainer CGI::checkRegFile(const StrContainer& cgiPath, Request &req)
{
	if (this->handler[0] == "$self") 
	{
		if (cgiPath.ends_with(this->cgiExt))
		{
			if (access(cgiPath.c_str(), X_OK) == 0)
			{
				this->prevURL = req.getUrl();
				this->prevExecPath = cgiPath;
				return (cgiPath);
			}
			else
				return ("403");
		}
		return "HandlePath";
	}
	else {
		if (!this->cgiExt.empty() && cgiPath.ends_with(this->cgiExt))
		{
			if (access(cgiPath.c_str(), R_OK) == 0)
			{
				this->prevURL = req.getUrl();
				this->prevExecPath = cgiPath;
				return (cgiPath);
			}
			else
				return ("403");
		}
        return "HandlePath";
	}
}

void CGI::createEnv(Request &req)
{
		int i = -1;
		std::vector<std::string> envp;
		envp.push_back("SCRIPT_FILENAME=" + this->scriptFilename);
		envp.push_back("SCRIPT_NAME=" + this->scriptName);
		if (!this->pathInfo.empty())
		{
			envp.push_back("PATH_INFO=" + this->pathInfo);
			envp.push_back("PATH_TRANSLATED=" + this->pathTranslated);
		}
		envp.push_back("DOCUMENT_ROOT=" + this->documentRoot);
        envp.push_back("QUERY_STRING=" + this->prevURL.getQuery());
        envp.push_back(("REQUEST_URI=" + URL::concatPaths("/", this->requestURI)));
        envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
        envp.push_back("REDIRECT_STATUS=200");
        envp.push_back("SERVER_NAME=" + this->prevURL.getDomain());
        envp.push_back("SERVER_PORT=" + this->prevURL.getPort());
        envp.push_back("SERVER_PROTOCOL=HTTP/1.1");
        envp.push_back("SERVER_SOFTWARE=webserv 1.0");
        envp.push_back("REQUEST_METHOD=" + getMethodString(req.getMethod()));
        envp.push_back("AUTH_TYPE=Basic");
        envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
        std::string result = req.getHeaders()["content-length"];
		if (result.empty())
			result = "0";
        envp.push_back("CONTENT_LENGTH=" + result);
        std::map<std::string, std::string> headers = req.getHeaders();
        for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
        {
                std::string key = it->first;
                if (key != ("content-type"))
                        key = "HTTP_" + key;
                std::transform(key.begin(), key.end(), key.begin(), ::toupper);
                std::replace(key.begin(), key.end(), '-', '_');
                std::string header = key + "=" + it->second;
                envp.push_back(header);
        }
        char **ev = new char*[envp.size() + 1];
        while (++i < (int)envp.size())
        {
                ev[i] = new char[envp[i].size() + 1];
                std::strcpy(ev[i], envp[i].c_str());
        }
        ev[i] = NULL;
        this->setEnv(ev);
}

StrContainer CGI::pathToScript(StrContainer cgiPath, const StrContainer& index, StrContainer filePath, Request &req, StrContainer route_path)
{
	StrContainer route_root = cgiPath; // Route root dir
	filePath = URL::removeFromStart(filePath, cgiPath);
	filePath = URL::removeFromStart(filePath, "/");
	StrContainer token;
	std::stringstream ss(filePath);
	while (std::getline(ss, token, '/')) {
		cgiPath = URL::concatPaths(cgiPath, token);
		route_path = URL::concatPaths(route_path, token);
		struct stat st;
		if (stat(cgiPath.c_str(), &st) == 0)
        {
			if (S_ISREG(st.st_mode)) {
				StrContainer result = this->checkRegFile(cgiPath, req);
				if (
					result.compare("403") && 
					result.compare("404") && 
					result.compare("HandlePath")
					)
					this->setupCGI(cgiPath, token, filePath, route_path, route_root); // setup basic variables for the env array
				return (result);
			}
			else if (!S_ISDIR(st.st_mode))
				return "403";
      	}
		else
			return "404";
	}
	cgiPath = URL::concatPaths(cgiPath, index);
	route_path = URL::concatPaths(route_path, index);
	filePath = URL::concatPaths(filePath, index);
	StrContainer result = this->checkRegFile(cgiPath, req);
	if (
		result.compare("403") && 
		result.compare("404") && 
		result.compare("HandlePath")
		)
		this->setupCGI(cgiPath, index, filePath, route_path, route_root); // setup basic variables for the env array
    return (this->checkRegFile(cgiPath, req));
}

void CGI::setupCGI(StrContainer cgiPath, StrContainer script, StrContainer filePath, StrContainer route_path, StrContainer route_root)
{
        this->pathInfo.clear();
        this->pathTranslated.clear();
        this->documentRoot.clear();

        // Clear previous entries

		this->scriptName = route_path;
		if (!route_root.starts_with("/"))
			this->scriptFilename = script;
		else
			this->scriptFilename = cgiPath;
		this->requestURI = filePath; // from scriptname onwards;
		this->documentRoot = route_root;
		size_t pos = filePath.find(script);
		if (pos != std::string::npos && pos + script.size() != filePath.size())
			this->pathInfo = filePath.substr(pos + script.size());
        if (!this->pathInfo.empty())
		{
            this->pathTranslated = this->documentRoot + ("/" + URL::removeFromEnd(filePath, script));
			this->pathTranslated.find_first_and_replace("/" + script, "");
		}
}

bool CGI::isEnabled() const
{
	return (this->enabled);
}
