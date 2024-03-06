#include "ConfigParser.hpp"
#include "Server.hpp"

 #include <iostream>
 #include <sstream>
 #include <cstdlib>


bool isNumber(std::string &str)
{
	for (long unsigned int i = 0; i < str.length(); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}


std::string&	ConfigParser::_extractSingleValueFromValueVector(const bool isRequired)
{
	if (_mulitValues.size() > 1)
	{
		_throwConfigError("Multiple values for key", 0, _key, true);
	}
	else if (isRequired && (_mulitValues.empty() || _mulitValues[0].empty()))
	{
		_throwConfigError("No value for key", 0, _key, true);
	}
	return (_mulitValues[0]);
}

void	ConfigParser::_processClientMaxBodySize()
{	
	std::string& numberString = _extractSingleValueFromValueVector(true);		
	std::istringstream iss(numberString);
    long long result;

	if (!std::isdigit(numberString[0]))
	{
		_throwConfigError("Only digits", 0, numberString, true);
	}
	else if (!(iss >> result) || result > 10000000000) // max size 10GB
	{
		_throwConfigError("Value out of range 0 - 10 GB", 0, numberString, true);
    }
	else if (iss.eof())
	{
		if (_currentServerConfig)
			_currentServerConfig->setMaxBodySize(result);		
    }
	else
	{
		_throwConfigError("Only digits", 0, numberString, true);
	}
}


void	ConfigParser::_processListen()
{
	std::string& numberString = _extractSingleValueFromValueVector(true);
	// std::cout << GREEN << "_processListen" << numberString << RESET << std::endl;
	std::stringstream ss(numberString);
	std::string buffer;
	std::string host;
	std::string port;
	size_t pos;
	int both = 0;
	while (ss >> buffer)
	{
		if ((pos = buffer.find(":")) != std::string::npos)
		{
			if (both)
				throw std::runtime_error("Host and port are already set!");
			host = buffer.substr(0, pos);
			port = buffer.substr(pos + 1, buffer.length());
			if (!isNumber(port))
				throw std::runtime_error("Host and port are already set!");
			if (host.empty())
				host = "127.0.0.1";
			else if (port.empty())
				port = "80";
			_currentServerConfig->setHosts(host, port);
			both = 1;
		}
		else if (isNumber(buffer))
		{
			if (both)
				throw std::runtime_error("Host and port are already set");
			_currentServerConfig->setHosts("127.0.0.1", buffer);
			both = 1;
		}		
		else
		{
			if (both)
				throw std::runtime_error("Host and port are already set!");
			_currentServerConfig->setHosts(buffer, "80");
			both = 1;
		}
	}
	// std::cout << GREEN << "Hosts: " << _currentServerConfig->printHosts() << RESET << std::endl;
}


void	ConfigParser::_processServerName()
{
    if (_mulitValues.empty()) {
        	
			_mulitValues.push_back("localhost");
    }
	if(_currentServerConfig->getServerNames().empty())
	{	
		for (std::vector<std::string>::iterator it = _mulitValues.begin(); it != _mulitValues.end(); ++it) 
		{
			std::string& serverName = *it;
			if (serverName.empty())	
			{	
				std::string localhost("localhost");
				std::stringstream ss(localhost);	
				_currentServerConfig->setServerNames(ss);
				break;
			}
			else 
			{
				std::stringstream ss(*it);
				_currentServerConfig->setServerNames(ss);
			}
		}
	}
}

void	ConfigParser::_processErrorPage()
{
	if (_mulitValues.size() == 0 || _oldParsingState == 1)
	{
		_throwConfigError("No error_page specified", 0, "", true);
	}
	else if (_mulitValues.size() != 2) {
       _throwConfigError("There must be 2 values for key: ", 0, _key, true);
    }
	else if (( (_mulitValues.size() == 2) && (_mulitValues[0].empty() || _mulitValues[1].empty()) ) || (_mulitValues.size() == 1) ) {
	   _throwConfigError("Empty value for key: ", 0, _key, true);
	}
	else
	{
		int num;
    	std::istringstream iss(_mulitValues[0]);
		if (!(iss >> num))	
			_throwConfigError("incorrect error page code: ", 0, _key, true);
 		else
		{	struct stat st;
			if (stat(_mulitValues[1].c_str(), &st) != 0)
				throw std::runtime_error("Wrong error page: " + _mulitValues[1]);
			_currentServerConfig->setErrorPage(num, _mulitValues[1]);
		}			
	}		
}

void	ConfigParser::_processDefaultErrorPage() //DELETE IT?
{	
}

void    ConfigParser::_processRoot()
{	
	std::string& str = _extractSingleValueFromValueVector(true);
	if (str.at(str.size() - 1) == '/')
		str.erase(str.size() - 1);
	std::ifstream dir(str.c_str());
	
    if (dir.is_open())
	{	
        if (dir.good()) {
            _currentLocationConfig->setRootDirectory(str);
        } else {
            _throwConfigError("directory in root location path not exist", 0, str, true);
        }
        dir.close();
	}

}

void    ConfigParser::_processLocationPath()
{   
	std::string& locationPath = _extractSingleValueFromValueVector(true);
	if (!_currentServerConfig->isLocationPathUnique(locationPath))
	  	_throwConfigError("Duplicate location path", 0, locationPath, true);
	_currentLocationConfig->setPath(locationPath);
}

void    ConfigParser::_processIndex()
{
	std::string& locationIndex = _extractSingleValueFromValueVector(true);
	if (_currentLocationConfig->getIndex() != "index.html")
		_throwConfigError("index already set", 0, _currentLocationConfig->getRootDir(), true);
	_currentLocationConfig->setIndex(locationIndex);
}

void    ConfigParser::_processCgiExtension()
{	
	if (_currentLocationConfig->isCgiEnabled())
		_throwConfigError("cgi already set up", 0, _currentLocationConfig->getRedirectUrl(), true);
	std::vector<std::string> handler;
	if(_mulitValues[0] == ".py" && _mulitValues.size() == 1)
	{
		handler.push_back("python3");		
		CGI cgi(handler, _currentLocationConfig->getEv());	
		_currentLocationConfig->setCGI(cgi);
		_currentLocationConfig->setType(CGI_);
		
		if (_currentLocationConfig->getCGIExt() != "")
			_throwConfigError("cgi_ext already set", 0, _currentLocationConfig->getCGIExt(), true);
		else
			_currentLocationConfig->getCGI().setCgiExt(".py");
	}
	else
	{
		_throwConfigError("Invalid CGI extension", 0, _mulitValues[0], true);
	}	
}


void    ConfigParser::_processUploadStore() {}

void	ConfigParser::_processReturn()
{
	if(_currentLocationConfig->getRedirectUrl() != "")
		_throwConfigError("return already set", 0, _currentLocationConfig->getRedirectUrl(), true);
	if (_mulitValues.size() == 0 || _oldParsingState == 1)
	{
		_currentLocationConfig->setRedirectStatusCode("0");
		_currentLocationConfig->setRedirectUrl("");
	}
	else if (_mulitValues.size() > 2) {
       _throwConfigError("There must be 2 values for key: ", 0, _key, true);
    }
	else if (( (_mulitValues.size() == 2) && (_mulitValues[0].empty() || _mulitValues[1].empty()) ) || (_mulitValues.size() == 1) ) {
	   _throwConfigError("Empty value for key: ", 0, _key, true);
	}
	else
	{	
		if(_mulitValues[0] != "301" && _mulitValues[0] != "302" && _mulitValues[0] != "303" && _mulitValues[0] != "307" && _mulitValues[0] != "308")
			_throwConfigError("Invalid status code", 0, _mulitValues[0], true);
		_currentLocationConfig->setRedirectStatusCode(_mulitValues[0]);
		_currentLocationConfig->setRedirectUrl(_mulitValues[1]);
	}
	_currentLocationConfig->setType(REDIRECTION_);	
}

void    ConfigParser::_processMethods()
{
	
	if ((_mulitValues[0].empty()))
		return;
	for (std::vector<std::string>::iterator it = _mulitValues.begin(); it != _mulitValues.end(); ++it)
	{
		if(*it == "GET")
			this->_currentLocationConfig->setAllowedMethod(get_method("GET"));
		else if(*it == "POST")
			this->_currentLocationConfig->setAllowedMethod(get_method("POST"));
		else if(*it == "DELETE")
			this->_currentLocationConfig->setAllowedMethod(get_method("DELETE"));
		else
			_throwConfigError("Only GET, POST and DELETE are allowed", 0, *it, true);
	}
}

void    ConfigParser::_processAutoindex()
{
	if(_currentLocationConfig->getDirListing() != false)
		_throwConfigError("autoindex already set", 0, _currentLocationConfig->getRedirectUrl(), true);
	const std::string& autoIndex = _extractSingleValueFromValueVector(true);
	if (autoIndex.empty())
		return;
	else if (autoIndex == "on")
		_currentLocationConfig->setDirListing(true);
	else if (autoIndex == "off")
		_currentLocationConfig->setDirListing(false);
	else
		_throwConfigError("autoindex must be on or off", 0, autoIndex, true);
}

