#include "ConfigParser.hpp"

bool ConfigParser::_isAllowedWs(char c)
{
	return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

bool ConfigParser::_isAllowedOws(char c)
{
	return (c == ' ' || c == '\t');
}

bool ConfigParser::_isCommentStart(char c)
{
	return (_lastChar != '\\' && c == '#');
}

bool ConfigParser::_isQuoteStart(char c)
{
	return (_lastChar != '\\' && c == '"');
}

bool ConfigParser::_isAllowedKeyChar(char c)
{
	return (isalnum(c) || c == '_');
}

bool ConfigParser::_isAllowedValueChar(char c)
{
	if ((_lastChar == '\\' || _isQuoteMode)
		&& (c == '{' || c == '}' || c == ' ' || c == '"' || c == ';'))
	{
		return true;
	}
	return (c >= 33 && c <= 126 && c != '{' && c != '}' && c != ';' && c != '"');
}

bool ConfigParser::_toggleQuoteMode(char c)
{
	if (_isUnescapedChar('"', c))
		_isQuoteMode = true;
	else if (c == '"' && _isQuoteMode && _lastChar != '\\')
		_isQuoteMode = false;
	else
		return false;
	return true;
}

bool ConfigParser::_isUnescapedChar(char expected, char actual)
{
	if (actual == expected && !_isQuoteMode && _lastChar != '\\')
		return true;
	return false;
}
