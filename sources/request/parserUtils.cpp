#include "RequestParser.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>



// -----
#include <stdexcept>

#include "logger/LoggerIncludes.hpp"

#include "Request.hpp"

bool RequestParser::_isHeaderNameChar(char c) //tchar
{
	static const std::string validChars = "!#$%&'*+-.^_`|~";
    return (std::isalnum(c) || validChars.find(c) != std::string::npos);
}

bool RequestParser::_isOWS(char c) //optional whitespace
{
	return (c == ' ' || c == '\t');
}

bool RequestParser::_isHeaderValueChar(char c) //vchar and OWS
{
	return ((c >= 33 && c <= 126) || _isOWS(c));
}

void RequestParser::_trimTrailingOWS(std::string& str)
{
	size_t pos = str.find_last_not_of(" \t");
	if (pos != std::string::npos)
	{
		str.erase(pos + 1);
	}
	else
	{
		str.clear();
	}
}

void RequestParser::_handleHttpMethod(const std::string& input)
{
  Method method;

  try
  {
    method = get_method(input);
  }
  catch (std::exception& e)
  {
    this->_statusCode = "400";
    throw std::runtime_error(RED + input + RESET + " unknown method");
  }

	if (method > DELETE)
  {
    this->_statusCode = "501";
    throw std::runtime_error(RED + input + RESET + " method not implemented");
  }
  this->request->setMethod(method);
}

void RequestParser::_handleHttpVersion(const std::string& input)
{
  std::string version = "";

  if (input.size() < 8 || input.compare(0, 5, "HTTP/") != 0)
    this->_statusCode = "400";
  else
    version = input.substr(5);

  if(version == "1.1" && this->_statusCode == "")
  {
    this->request->setVersion(input);
    return ;
  }
  else if (this->_statusCode != ""
          && (version == "0.9"
              || version == "1.0"
              || version == "2.0"
              || version == "3.0"))  
  {
    this->_statusCode = "505";
    throw std::runtime_error(RED + input + RESET + " unsupportet HTTP version");
  }
  else
  {
    this->_statusCode = "400";
    throw std::runtime_error(RED + input + RESET + " invalid HTTP version");
  }
}
