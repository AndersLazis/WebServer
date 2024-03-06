#include "RequestParser.hpp"

#include <stdexcept>


#include "UriParser.hpp"

#include "Request.hpp"

#include "logger/LoggerIncludes.hpp"

void RequestParser::_changeState(int current, int next)
{
   (void)current;
   this->parsingState = next;
}

/* ================================ request Line ================================== */
void	RequestParser::_handleStateMethod(char c)
{
	if (c == ' ')
	{
		_handleHttpMethod(_headerValue);
		_paramterLength = 0;
		_headerValue.clear();
		_changeState(STATE_METHOD, STATE_URI);
	}
	else if (_paramterLength < RequestParser::METHOD_MAX_LENGTH
	         && std::isupper(c))
	{
		_paramterLength++;
		_headerValue.push_back(c);
	}
	else // Header method is too long || invalid character
	{
		this->_statusCode = "400";
		throw std::runtime_error("Header method is too long || invalid character: "
														 + this->_headerValue);
	}
}

void	RequestParser::_handleStateUri(char c)
{
	if (c == '\r' && _lastChar != '\r')
	{
		// Do nothing, expect line feed next
	}
	else if (c == '\n' && _lastChar == '\r')
	{
		this->_statusCode = "400";
		if (this->_headerValue.empty())
			throw std::runtime_error("Missing Uri");
		else
			throw std::runtime_error("Missing Http version");
	}
	else if (c == ' ')
	{
		// this->request->setUrl(URL(this->_headerValue));
		UriParser uriParser;
		URL				url;
		uriParser.setUrl(url);
		if (!uriParser.parse(_headerValue))
		{
			this->_statusCode = "400";
			throw std::runtime_error("Invalid uri");
		}
		else
		{
			this->request->setUrl(url);
		}
		_paramterLength = 0;
		_headerValue.clear();
		_changeState(STATE_URI, STATE_HTTP_VERSION);
	}
	else if (_paramterLength < RequestParser::URI_MAX_LENGTH)
	{
		_paramterLength++;
		_headerValue.push_back(c);
	}
	else if (_paramterLength > RequestParser::URI_MAX_LENGTH)
	{
		this->_statusCode = "414";
		throw std::runtime_error("Uri too long: " + this->_headerValue);
	}
	else
	{
		//Invalid character in header uri
		this->_statusCode = "400";
		throw std::runtime_error("Invalid character in header uri: "
														 + this->_headerValue);
	}
}

void	RequestParser::_handleStateHttpVersion(char c)
{
	if (c == '\r' && _lastChar != '\r')
	{
		// Do nothing, expect line feed next
	}
	else if (c == '\n' && _lastChar == '\r')
	{
		_handleHttpVersion(this->_headerValue);
		_paramterLength = 0;
		_headerValue.clear();
		_changeState(STATE_HTTP_VERSION, STATE_NAME);
	}
	else if (_paramterLength < RequestParser::HTTP_VERSION_MAX_LENGTH)
	{
		_paramterLength++;
		_headerValue.push_back(c);
	}
	else
	{
		this->_statusCode = "400";
		throw std::runtime_error("Http version too long: " + this->_headerValue);
	}
}

/*---------------------header fields parsing----------------*/

void	RequestParser::_handleStateFieldName(char c)
{
	if (c == '\r' && _lastChar != '\r' && _headerName.empty())
	{
		// Do nothing, expect line feed next
		// this->log << LOG_DEBUG << "Field name doing nothing" << "\n";
	}
	else if (c == '\n' && _lastChar == '\r' && _headerName.empty())
	{
		// End of line, transition states
		// this->log << LOG_DEBUG << "End of header fields" << "\n";
		_changeState(STATE_NAME, STATE_END);
	}
	else if (c == ':' && !_headerName.empty())
	{
		_changeState(STATE_NAME, STATE_OWS);
	}
	else if (_isHeaderNameChar(c) && _paramterLength < RequestParser::MAX_FIELD_LENGTH)
	{
		_paramterLength++;
		_headerName.push_back(std::tolower(c));
	}
	else if (_paramterLength > RequestParser::MAX_FIELD_LENGTH)
	{
		this->_statusCode = "431";
		throw std::runtime_error("To many header fields in Name");
	}
	else
	{
		this->_statusCode = "400";
		throw std::runtime_error("Empty or wrong char in header name");
	}
}

void	RequestParser::_handleStateOWS(char c)
{
	if (_isOWS(c))
	{
		// Skip leading whitespace in header value
	}
	else if (!_isOWS(c) && _isHeaderValueChar(c) && _paramterLength < RequestParser::MAX_FIELD_LENGTH) //TODO: define a max len is it for all or jsut one? Currently it is for one
	{
		_paramterLength++;
		_headerValue.push_back(c);  // Start of header value
		_changeState(STATE_OWS, STATE_VALUE);  // Transition to value state
	}
	else if (_paramterLength >= RequestParser::MAX_FIELD_LENGTH)
	{
		this->_statusCode = "431";
		throw std::runtime_error("To many header fields in OWS");
	}
	else if (c == '\r' && _lastChar != '\r')
	{
		// Do nothing, expect line feed next
	}
	else if (c == '\n' && _lastChar == '\r' && !_headerValue.empty())
	{
		// End of line, transition states
		_changeState(STATE_OWS, STATE_FIELDS_PAIR_DONE);
	}
	else
	{
		this->_statusCode = "400";
		throw std::runtime_error("Empty or wrong char in header field value");
	}
}

void	RequestParser::_handleStateFieldValue(char c)
{
	if (c == '\r' && _lastChar != '\r')
	{
		// Do nothing, expect line feed next
	}
	else if (c == '\n' && _lastChar == '\r' && !_headerValue.empty())
	{
		// End of line, transition states
		_storeFieldPair();
		_headerName.clear();
		_headerValue.clear();
		_paramterLength = 0;
		_changeState(STATE_VALUE, STATE_NAME);
	}
	else if (_isHeaderValueChar(c) && _paramterLength < RequestParser::MAX_FIELD_LENGTH)
	{
		_paramterLength++;
		_headerValue.push_back(c);
	}
	else if (_paramterLength > RequestParser::MAX_FIELD_LENGTH)
	{
		this->_statusCode = "431";
		throw std::runtime_error("To many header fields in Value");
	}
	else
	{
		this->_statusCode = "400";
		throw std::runtime_error("Empty or wrong char in header field value");
	}
}

bool	isOnlyOnceAllowed(const std::string& _headerName)
{
	return (_headerName == "host"
	       || _headerName == "authorization"
				 || _headerName == "user-agent");
}

void	RequestParser::_storeFieldPair(void)
{
	_trimTrailingOWS(_headerValue);
	if (_headerValue.empty() && _headerName != "host")
	{
		this->_statusCode = "400";
		throw std::runtime_error("Empty header value");
	}
	// std::cout << "_headerName: " << _headerName << std::endl;
	// std::cout << "_headerValue: " << _headerValue << std::endl;
	// std::cout << "_httpMethod: " << _httpMethod << std::endl;

	if (isOnlyOnceAllowed(_headerName)
			&& this->request->headers.find(_headerName) != this->request->headers.end())
	{
		this->_statusCode = "400";
		throw std::runtime_error("Duplicate header field");
	}
	this->request->setHeader(this->_headerName, this->_headerValue);
}

void	RequestParser::_handleStateFieldPair(char c)
{
	if (c == '\r' && _lastChar != '\r')
	{
		// Do nothing, expect line feed next
	}
	else if (c == '\n' && _lastChar == '\r')
	{
		// End of line, transition states
		_changeState(STATE_FIELDS_PAIR_DONE, STATE_END);
	}
	else
	{
		this->_statusCode = "400";
		throw std::runtime_error("Invalid character after header pair");
	}
}
