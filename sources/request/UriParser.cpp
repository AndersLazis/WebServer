#include "UriParser.hpp"

#include <iostream>
#include <cstdlib>

#include "RequestParser.hpp"

// ----
#include "logger/LoggerIncludes.hpp"
#include <sstream>
#include <stdexcept>

UriParser::UriParser()
	: _url(NULL),
	  log(Logger::instance(), "UriParser"),
	  _parsingState(UriParser::STATE_START),
	  _uriIndex(0), _uriInput(NULL)
{
	this->_initParsingStateHandlers();
}

UriParser::UriParser(const UriParser& other)
         : log(Logger::instance(), "UriParser")
{
	this->_initParsingStateHandlers();
	*this = other;
}

UriParser& UriParser::operator=(const UriParser& other)
{
	if (this != &other)
	{
		this->_url = other._url;
		this->log = other.log;
		this->_parsingState = other._parsingState;
		this->_uriIndex = other._uriIndex;
		this->_uriInput = other._uriInput;
		this->_value = other._value;
	}
	return *this;
}

UriParser::~UriParser(void) {}

void UriParser::_initParsingStateHandlers()
{
	parsingStateHandlers[0] = &UriParser::_handleStateStart;
	parsingStateHandlers[1] = &UriParser::_handleStateScheme;
	parsingStateHandlers[2] = &UriParser::_handleStateDomain;
	parsingStateHandlers[3] = &UriParser::_handleStatePath;
	parsingStateHandlers[4] = &UriParser::_handleStateQuery;
}

void UriParser::setUrl(URL& url)
{
	this->_url = &url;
}

bool	UriParser::parse(const std::string& uriInput)
{
	this->_uriInput = &uriInput;
	try
	{
		while (this->_uriIndex < this->_uriInput->length())
		{
			char c = uriInput[this->_uriIndex];

			if (c == '%')
				c = _decodePercentEncodedChar(*this->_uriInput);
			else if (c == '+')
				c = ' ';

			(this->*parsingStateHandlers[this->_parsingState])(c);
			// this->log << LOG_DEBUG
			// 									<< "Parsing state: " << this->_parsingState
			// 									<< " | Value: " << this->_value << "\n";

			this->_uriIndex++;
		}
	}
	catch(const std::exception& e)
	{
		this->log << LOG_DEBUG
											  << "Error while parsing uri: "
											  << e.what() << "\n";
		return (false);
	}
	switch (this->_parsingState)
	{
		case UriParser::STATE_START:
			break;
		case UriParser::STATE_PATH:
			this->_url->setPath(this->_value);
			break;
		case UriParser::STATE_QUERY:
			this->_url->setQuery(this->_value);
			break;
		default:
		{
			this->log << LOG_WARN
												<< "Unhandled parsing state: " 
												<< this->_parsingState << "\n";
		}
	}
	this->_value.clear();
	if (this->_url->getPath().empty())
	{
		this->log << LOG_INFO
												<< "Empty path in URI: " << uriInput << "\n";
		return (false);
	}
	return (true);
}

void 	UriParser::_changeState(UriParser::States state, UriParser::States nextState)
{
	(void)state;
	this->_parsingState = nextState;
}

void	UriParser::_handleStateStart(char c)
{
	if (c == '/')
	{
		_changeState(UriParser::STATE_START, UriParser::STATE_PATH);
		(this->*parsingStateHandlers[UriParser::STATE_PATH])(c);
	}
	else if (c == 'h')
	{
		_changeState(UriParser::STATE_START, UriParser::STATE_SCHEME);
		(this->*parsingStateHandlers[UriParser::STATE_SCHEME])(c);
	}
	else
	{
		throw std::runtime_error("Invalid URI start: " + c);
	}
}

void	UriParser::_handleStateScheme(char c)
{
	if (c == ':'
			&& !this->_value.empty()
			&& this->_uriInput->compare(this->_uriIndex, 3, "://") == 0
			&& this->_value == "http")
	{
		this->_uriIndex += 2; // Skip '://'
		this->_value.clear();
		_changeState(UriParser::STATE_SCHEME, UriParser::STATE_DOMAIN);
	}
	else if (_isValidSchemeChar(c))
	{
		this->_value += c;
	}
	else
	{
		throw std::runtime_error("Invalid scheme: " + this->_value);
	}
}

void	UriParser::_handleStateDomain(char c)
{
	if (c == '/')
	{
		this->_url->setDomain(this->_value);
		this->_value.clear();
		_changeState(UriParser::STATE_DOMAIN, UriParser::STATE_PATH);
		(this->*parsingStateHandlers[UriParser::STATE_PATH])(c);
	}
	else if (_isValidDomainChar(c))
	{
		this->_value += std::tolower(c);
	}
	else
	{
		throw std::runtime_error("Invalid character in domain: " + this->_value);
	}
}

void	UriParser::_handleStatePath(char c)
{
	if (c == '?')
	{
		this->_url->setPath(this->_value);
		this->_value.clear();
			_changeState(UriParser::STATE_PATH, UriParser::STATE_QUERY);
	}
	else if (_isValidPathChar(c))
	{
		this->_value += c;
	}
	else
	{
		throw std::runtime_error("Invalid character in path: " + this->_value);
	}
}

void	UriParser::_handleStateQuery(char c)
{
	if (_isValidPathChar(c))
	{
		this->_value += c;
	}
	else
	{
		throw std::runtime_error("Invalid character in query: " + this->_value);
	}
}

char	UriParser::_decodePercentEncodedChar(const std::string &uriInput)
{
	char decodedChar = '%';
	std::string hexString = "";
	if (this->_uriIndex + 2 < uriInput.length())
	{
		hexString = uriInput.substr(this->_uriIndex + 1, 2);
		std::istringstream issHex(hexString);
		int hexValue;
		if (issHex >> std::hex >> hexValue && issHex.eof())
		{
			decodedChar = static_cast<char>(hexValue);
			this->_uriIndex += 2;
		}
		return decodedChar;
	}
	
	throw std::runtime_error("Invalid percent-encoded character: " + hexString);
	return decodedChar;
}

bool UriParser::_isValidSchemeChar(char c)
{
	return isalnum(c) || c == '+' || c == '-' || c == '.';
}

bool UriParser::_isValidDomainChar(char c)
{
	return isalnum(c) || c == '-' || c == '.';
}

bool UriParser::_isValidPathChar(char c)
{
	return isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~' || c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=' || c == ':' || c == '@' || c == '/';
}
