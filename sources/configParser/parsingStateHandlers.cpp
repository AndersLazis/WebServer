 #include "ConfigParser.hpp"
 #include "Server.hpp"

// #include "WebServerConfig.hpp"
// #include "ServerConfig.hpp"
// #include "LocationConfig.hpp"

void ConfigParser::_changeState(int current, int next)
{
	this->_oldParsingState = current;
	this->_parsingState = next;
}

void	ConfigParser::_handleStateWs(char c) // STATE 0; after value, after Block starts and ends
{ 	
 	if (_isCommentStart(c))
 	{
 		_changeState(ConfigParser::STATE_WS, ConfigParser::STATE_COMMENT);
 	}
	else if (_isAllowedKeyChar(c))
	{
		_addCharToKey(c);
		_changeState(ConfigParser::STATE_WS, ConfigParser::STATE_KEY);
	}
 	else if (_isUnescapedChar('{', c)) //start of block
 	{		
			// std::cout << "_key:" << _key << "\"    !_currentServerConfig:" << _currentServerConfig <<std::endl;
  		if (_key == "server" && !_currentServerConfig) //start of server
  		{
  			_currentServerConfig = new Server;
			_currentServerConfig->setEnv(this->getEnv());
  		}
  		else if (_key == "location" && _currentServerConfig && !_currentLocationConfig) //start of location, after location path
  		{
 			_currentLocationConfig = new Route;
 			_currentLocationConfig->setEv(_currentServerConfig->getEnv());
 			_processLocationPath(); //validation of path
  		}
 		else if ((_key == "location" && (!_currentServerConfig || _currentLocationConfig)) //location outside of server or inside of location
 				|| (_key == "server" && _currentServerConfig)) //server inside of server
 		{
 			_throwConfigError("Unexpected opening of block", 0, _key, true); //unexpected opening of block
 		}
 		else
 		{	
 			_throwConfigError("Unexpected", c, "", true); //unexpected {
 		}
  		_key.clear();
 		_value.clear();
  		_mulitValues.clear();
 		_paramterLength = 0;
  	}
  	else if (_isUnescapedChar('}', c)) //end of block
  	{
 		if (_currentLocationConfig) //end of location
 		{
 			_validateLocationConfig(_currentLocationConfig);
 			_addLocationConfig(_currentLocationConfig);
 			_currentLocationConfig = NULL;
 			_resetKeyCounts(_locationKeys);
			//delete _currentLocationConfig;
 		}
 		else if (_currentServerConfig) //end of server
 		{
 			_validateServerConfig(_currentServerConfig);
 			_addServerConfig(_currentServerConfig);
 			_currentServerConfig = NULL;
 			_resetKeyCounts(_serverKeys);
 		}
 		else
 		{
 			_throwConfigError("Unexpected", c, "", true); //unexpected closing of block
 		}
  	}
  	else if (_isAllowedWs(c))
  	{
  		return;
  	}
  	else
  	{
  		_throwConfigError("Unexpected", c, "", true);
  	}
}

 void ConfigParser::_handleStateKey(char c) //state 1
 {	//std::cout << "_isAllowedOws(c):" << _isAllowedOws(c) << "  _key == \"location\":" << (_key) <<  "   _currentServerConfig:" << _currentServerConfig <<    "!_currentLocationConfig:" <<  _currentLocationConfig << std::endl;
	if (_isCommentStart(c) && _key != "server")
	{
		_throwConfigError("Comment in key", c, _key, true);
	}
	else if (_isAllowedKeyChar(c))
	{	
		_addCharToKey(c);
	}
	else if (_isAllowedOws(c) && _key == "location" && _currentServerConfig && !_currentLocationConfig) //start of location path
	{
		_paramterLength = 0;
		_changeState(ConfigParser::STATE_KEY, ConfigParser::STATE_LOCATION);
	}
	else if (_key == "server")
	{
		_paramterLength = 0;
		_changeState(ConfigParser::STATE_KEY, ConfigParser::STATE_WS);
		_handleStateWs(c);
	}
	else if (_isUnescapedChar(':', c))
	{
		//validate key is appropriate for current block -> move this check to when values are gathered
		_changeState(ConfigParser::STATE_KEY, ConfigParser::STATE_OWS);
	}
	else
	{
		_throwConfigError("Invalid character in key", c, _key, true);
	}
 }

void	ConfigParser::_handleStateOws(char c)	// state 3
{	
	if (_isUnescapedChar(';', c))
	{	
		_processKeyValuePair();
	}
	else if (_isCommentStart(c))
	{
		_throwConfigError("Missing semicolon", c, "", true);
	}
	else if (_isAllowedValueChar(c))
	{
		_addCharToValue(c);
		_changeState(ConfigParser::STATE_OWS, ConfigParser::STATE_VALUE);
	}
	else if (_isAllowedOws(c))
	{
		return;
	}
	else if (_isUnescapedChar('"', c))
	{
		_isQuoteMode = true;
		_changeState(ConfigParser::STATE_OWS, ConfigParser::STATE_VALUE);
	}
	else
	{
		_throwConfigError("Missing semicolon", c, "", true);
	}
}

void	ConfigParser::_handleStateValue(char c) // state 3
{
//std::cout << "!===value: " << (int)c << std::endl;
	if (_isUnescapedChar(';', c))
	{
		_processKeyValuePair();
	}
	else if (_toggleQuoteMode(c))
	{
		return;
	}
	else if (_isAllowedValueChar(c))
	{
		_addCharToValue(c);
	}
	else if (_isAllowedOws(c))
	{
		if (!_value.empty())
		{	
			_mulitValues.push_back(_value);
			_value.clear();
		}
		_changeState(ConfigParser::STATE_VALUE, ConfigParser::STATE_OWS);
	}
	else
	{
		_throwConfigError("Invalid character in value", c, _value, true);
	}
}

void ConfigParser::_handleStateComment(char c)
{	//(void)c;
	if (c == '\n')
		_changeState(ConfigParser::STATE_COMMENT, this->_oldParsingState);
	else
		_changeState(this->_oldParsingState, ConfigParser::STATE_COMMENT);
}

 void ConfigParser::_handleStateLocation(char c)
 {	//(void)c;
	if (_isUnescapedChar('{', c)) // start of location block after location path
	{
		_mulitValues.push_back(_value);
		_changeState(ConfigParser::STATE_LOCATION, ConfigParser::STATE_WS);
		_handleStateWs(c);
	}
	else if (_toggleQuoteMode(c))
	{
		return;
	}
	else if (_isCommentStart(c) && _value.empty())
	{
		_throwConfigError("Comment in location path", c, _key, true);
	}
	else if (_isCommentStart(c) && !_value.empty())
	{
		_mulitValues.push_back(_value);
		_changeState(ConfigParser::STATE_WS, ConfigParser::STATE_COMMENT);
	}
	else if (_isAllowedValueChar(c))
	{
		_addCharToValue(c);
	}
	else if (_isAllowedOws(c) && _value.empty())
	{
		return;
	}
	else if (!_value.empty() && _isAllowedWs(c)) 
	{	
		_mulitValues.push_back(_value);
		_changeState(ConfigParser::STATE_LOCATION, ConfigParser::STATE_WS);
	}
	else
	{
		_throwConfigError("Invalid location path", c, _key, true);
	}
 }
