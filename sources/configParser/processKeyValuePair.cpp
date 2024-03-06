 #include "ConfigParser.hpp"

void	ConfigParser::_processKeyValuePair(void)
{
	 _mulitValues.push_back(_value);
	 _validateAndHandleKey();
	 _paramterLength = 0;
	 _key.clear();
	 _value.clear();
	 _mulitValues.clear();
	_changeState(STATE_OWS, STATE_WS);
}

void	ConfigParser::_validateAndHandleKey(void)
{
	if (!_currentServerConfig && !_currentLocationConfig)
	{
		_validateKeyAndCallHandler(_httpKeys);
	}
	else if (_currentServerConfig && !_currentLocationConfig)
	{
		_validateKeyAndCallHandler(_serverKeys);
	}
	else if (_currentServerConfig && _currentLocationConfig)
	{
		_validateKeyAndCallHandler(_locationKeys);
	}
	else
	{	
		_throwConfigError("Invalid key", 0, _key, true);
	}
}

void	ConfigParser::_validateKeyAndCallHandler(std::map<std::string, std::pair<int, _processValueFunctions> >& keys)
{
	std::map<std::string, std::pair<int, ConfigParser::_processValueFunctions> >::iterator it = keys.find(_key);
	if (it == keys.end())
	{
		_throwConfigError("Invalid key", 0, _key, true);
	}
	else if (it->second.first++ != 0 && it->first != "error_page")
	{
		_throwConfigError("Duplicate key", 0, _key, true);
	}
	else
	{
		ConfigParser::_processValueFunctions functionPointer = it->second.second;
		(this->*functionPointer)();
	}
}
