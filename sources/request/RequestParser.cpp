#include "RequestParser.hpp"

#include <iostream>


// ------------
#include "RequestReceiver.hpp"
#include "logger/LoggerIncludes.hpp"

//TODO: decide for size limit of header fields
RequestParser::RequestParser()
             : parsingState(STATE_METHOD),
               request(NULL),
               log(Logger::instance(), "RequestParser"),
               _positionInInput(0), _paramterLength(0), _lastChar('\0'),
               _statusCode("")
{
   this->_initializeParsingStateHandlers();
}

RequestParser::RequestParser(const RequestParser& other)
             : log(Logger::instance(), "RequestParser")
{
   this->_initializeParsingStateHandlers();
   *this = other;
}

RequestParser& RequestParser::operator=(const RequestParser& other)
{
   if (this != &other)
   {
      this->_positionInInput = other._positionInInput;
      this->_paramterLength = other._paramterLength;
      this->request = other.request;
      this->log = other.log;
      this->_lastChar = other._lastChar;
      this->parsingState = other.parsingState;
   }
   return *this;
}

RequestParser::~RequestParser(void) {}

void RequestParser::_initializeParsingStateHandlers()
{
   _parsingStateHandlers[RequestParser::STATE_METHOD]             = &RequestParser::_handleStateMethod;
   _parsingStateHandlers[RequestParser::STATE_URI]                = &RequestParser::_handleStateUri;
   _parsingStateHandlers[RequestParser::STATE_HTTP_VERSION]       = &RequestParser::_handleStateHttpVersion;
   _parsingStateHandlers[RequestParser::STATE_NAME]               = &RequestParser::_handleStateFieldName;
   _parsingStateHandlers[RequestParser::STATE_OWS]                = &RequestParser::_handleStateOWS;
   _parsingStateHandlers[RequestParser::STATE_VALUE]              = &RequestParser::_handleStateFieldValue;
   _parsingStateHandlers[RequestParser::STATE_FIELDS_PAIR_DONE]   = &RequestParser::_handleStateFieldPair;
}

std::string   RequestParser::parseRequestChunk(const char* input, size_t inputLength, bool resetPosition)
{
   if (resetPosition)
      this->_positionInInput = 0;
   try
   {
      while (_positionInInput < inputLength && parsingState != STATE_END)  //TODO: check if the ending condition is correct
      {
         // std::cout << "current state: " << parsingState << std::endl;
         // std::cout << "current char: " << input[_positionInInput] << std::endl;
         _parseOneChar(input[_positionInInput]);
         _lastChar = input[_positionInInput];
         _positionInInput++;
      }
   }
   catch (const std::exception& e)
   {
      Logger::instance() << LOG_DEBUG
                          << "Error while parsing request: "
                          << e.what() << "\n";
   }
   return this->_statusCode;
}

void RequestParser::_parseOneChar(char input)
{
  (this->*_parsingStateHandlers[parsingState])(input);
}

size_t RequestParser::getAmountParsed(void) const
{
   return (this->_positionInInput);
}

void RequestParser::setRequest(Request& request)
{
   this->request = &request;
}
