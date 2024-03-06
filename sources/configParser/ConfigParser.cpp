#include "ConfigParser.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include "Manager.hpp"
#include "LocationConfig.hpp"

ConfigParser::ConfigParser()
			:	_currentServerConfig(NULL),
				_currentLocationConfig(NULL),
				_parsingState(STATE_WS),
				_oldParsingState(STATE_WS),
				_key(""),
				_value(""),
				_paramterLength(0),
				_lastChar('\0'),
				_isQuoteMode(false),
				_lineCount(1),
				_charCount(0)			
{

	parsingStateHandlers[STATE_WS]			= &ConfigParser::_handleStateWs;
	parsingStateHandlers[STATE_KEY]			= &ConfigParser::_handleStateKey;
	parsingStateHandlers[STATE_OWS]			= &ConfigParser::_handleStateOws;
	parsingStateHandlers[STATE_VALUE]		= &ConfigParser::_handleStateValue;
	parsingStateHandlers[STATE_COMMENT]		= &ConfigParser::_handleStateComment;
	parsingStateHandlers[STATE_LOCATION]	= &ConfigParser::_handleStateLocation;

 	_httpKeys["error_page"]             = std::make_pair(0, &ConfigParser::_processDefaultErrorPage);

 	_serverKeys["client_max_body_size"] = std::make_pair(0, &ConfigParser::_processClientMaxBodySize);
 	_serverKeys["error_page"]           = std::make_pair(0, &ConfigParser::_processErrorPage);				
 	_serverKeys["listen"]               = std::make_pair(0, &ConfigParser::_processListen);				
 	_serverKeys["server_name"]          = std::make_pair(0, &ConfigParser::_processServerName);			
 	_serverKeys["location"]             = std::make_pair(0, &ConfigParser::_processLocationPath);	

 	_locationKeys["root"]               = std::make_pair(0, &ConfigParser::_processRoot);					
 	_locationKeys["index"]              = std::make_pair(0, &ConfigParser::_processIndex);					
 	_locationKeys["cgi_extension"]      = std::make_pair(0, &ConfigParser::_processCgiExtension);				
 	_locationKeys["upload_store"]       = std::make_pair(0, &ConfigParser::_processUploadStore);			
 	_locationKeys["return"]             = std::make_pair(0, &ConfigParser::_processReturn);			
 	_locationKeys["allow_methods"]      = std::make_pair(0, &ConfigParser::_processMethods);			
 	_locationKeys["autoindex"]          = std::make_pair(0, &ConfigParser::_processAutoindex);				
}

ConfigParser::~ConfigParser()
{
	delete _currentServerConfig;
	_currentServerConfig = NULL;
	delete _currentLocationConfig;
	_currentLocationConfig = NULL;
}

std::string createFile(std::ifstream &conf)
{
	std::string	line;
	std::string	server;

	while (conf.good())
	{
		getline(conf, line);
		server.append(line);
		server.append("\n");
	}
	return server;
}

bool checkBrackets(std::string server)
{
	int		scope;
	int		i;

	scope = 0;
	i = 0;
	while (server[i])
	{
		if (server[i] == '{')
		{
			scope++;
			if (scope % 2 == 1 && server[i] != '{')
				return false;
		}
		else if (server[i] == '}')
		{
			scope--;
			if (scope % 2 == 0 && server[i] != '}')
				return false;
		}
		if (scope < 0)
			return false;
		i++;
	}
	return !scope;
}


void ConfigParser::_throwConfigError(const std::string& message, char offendingChar, const std::string& offendingString, bool givePosition)
{
	std::stringstream ss;

	ss << "Error: {" << message << "}";
	if (givePosition)
		ss << " at line {" << _lineCount << "}";
	if (offendingChar)
		ss << " at character {" << _charCount << "}" << " with char {" << offendingChar << "}";
	if (!offendingString.empty())
		ss << " with string {" << offendingString << "}";
	
	throw std::runtime_error(ss.str());
}

void ConfigParser::parseConfig(const std::string& configPath)
 {
	std::ifstream configFile;
	std::string buffer;
	if (!_isFileNameValid(configPath))
	 	throw std::runtime_error("Error: Invalid config file name");
	configFile.open(configPath.c_str(), std::ifstream::in);
	if (!configFile.is_open())
		throw std::runtime_error("Error: Could not open config file");

	std::ifstream	conf(configPath.c_str());
	std::string	server;
	server = createFile(conf);
	if (checkBrackets(server) == false)
		_throwConfigError("Invalid Brackets", 'x', "", true);

	buffer.reserve(ConfigParser::BUFFER_SIZE);
	while (configFile.good())
	{
	 	configFile.read(&buffer[0], ConfigParser::BUFFER_SIZE - 1);
		if (configFile.bad())
	 		break;
	 	std::streamsize bytesRead = configFile.gcount();
	 	buffer[bytesRead] = '\0';
	 	if (bytesRead)
	 	{
	 		for (std::streamsize i = 0; i < bytesRead; i++)
	 		{
	 			_parseOneChar(buffer[i]);
	 			_lastChar = buffer[i];
			}
	 	}
	}
	if (!configFile.eof())
		throw std::runtime_error("Error: Could not read config file");
	if (_lastChar == '\\')
		throw std::runtime_error("Error: '\\' is last char in config file");
	configFile.close();
 }




bool	ConfigParser::_isFileNameValid(const std::string& fileName)
{
	if (fileName.size() < 5)
		return (false);
	return (fileName.compare(fileName.size() - 5, 5, ".conf") == 0);
}

void ConfigParser::_parseOneChar(char c)
{
 	if (c == '\n')
 	{
 		_lineCount++;
 		_charCount = 0;
 	}
	else
	{
		_charCount++;
	}
	if (_lastChar != '\\' && c == '\\')
		return;
	(this->*parsingStateHandlers[_parsingState])(c);
	if (_lastChar == '\\' && c == '\\')
		_lastChar = '\0';
}

void ConfigParser::_addCharToKey(char c)
{
	if (_paramterLength < MAX_KEY_LENGTH)
	{
		_paramterLength++;
		_key.push_back(std::tolower(c));
	}
	else
	{
		_throwConfigError("Key too long", 0, _key, true);
	}
}

void ConfigParser::_addCharToValue(char c)
{
	if (_paramterLength < MAX_VALUE_LENGTH)
	{
		_paramterLength++;
		_value.push_back(c);
	}
	else
	{
		_throwConfigError("Value too long", 0, _value, true);
	}
}

void    ConfigParser::_validateLocationConfig(Route* currentLocationConfig)
{ 
	// /* Check location path */
	if(currentLocationConfig->getRootDir() == "")
	  	_throwConfigError("Location root dir not set", 0, "", true);	
	CGI cgi = currentLocationConfig->getCGI();
	if(cgi.getCgiExt() == "")
	{
		cgi.setCgiExt(".py");		
	}
	currentLocationConfig->setCGI(cgi);
}

void    ConfigParser::_validateServerConfig(Server* currentServerConfig)
{	
	// std::cout << GREEN <<  "ConfigParser::_validateServerConfig" << RESET << std::endl;
	std::multimap<std::string,std::string>	hosts = currentServerConfig->getHosts();
	for (std::multimap<std::string, std::string>::iterator it = hosts.begin(); it != hosts.end(); ++it) 
	{	
		// std::cout << "Host: " << it->first << " Port: " << it->second << std::endl;
        if (it->first.empty() || it->second.empty())
            throw std::runtime_error("Error: Server has empty Ip adress or port\n");
		if (it->first == "0" || it->second == "0")      
       		throw std::runtime_error("Error: Server has no valid Ip adress or port\n");
    }			
}

void	ConfigParser::_resetKeyCounts(std::map<std::string, std::pair<int, _processValueFunctions> >& keys)
{
	std::map<std::string, std::pair<int, _processValueFunctions> >::iterator it = keys.begin();
	for( ; it != keys.end(); ++it)
	{
		it->second.first = 0;
	}
}


void ConfigParser::_printconfig()
{
	std::cout << BLUE << "------------ * PRINT CONFIG * -----------" << RESET << std::endl;
	for(std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		it->printServer();
	}
	std::cout << std::endl << BLUE << "------------ * END PRINT CONFIG * -----------" << RESET << std::endl;
}


void            ConfigParser::_addServerConfig(Server* currentServerConfig)
{
	this->servers.push_back(*currentServerConfig);
	delete _currentServerConfig;
	_currentServerConfig = NULL;
}

void            ConfigParser::_addLocationConfig(Route* currentLocationConfig)
{
	if(_currentServerConfig)
		_currentServerConfig->setRoute(*currentLocationConfig);
	delete _currentLocationConfig;
	_currentLocationConfig = NULL;
}


 std::vector<Server> ConfigParser::getServers() const
{
	return this->servers;
}

void ConfigParser::setEnv(char **env)
{
	this->env = env;
}

char** ConfigParser::getEnv() const
{
	return this->env;
}