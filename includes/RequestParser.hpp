#ifndef REQUEST_PARSER_HPP_INCLUDED
# define REQUEST_PARSER_HPP_INCLUDED

# include <string>
# include <map>
# include <vector>
# include <sys/types.h>


// --------

class Request;

#include "Method.hpp"
#include "logger/LoggerIncludes.hpp"

class RequestParser {
public:
   enum States
   {
      STATE_METHOD,
      STATE_URI,
      STATE_HTTP_VERSION,
      STATE_NAME,
      STATE_OWS,
      STATE_VALUE,
      STATE_FIELDS_PAIR_DONE,
      STATE_END //Body reached, or End of request
   };

   int                          parsingState;

   RequestParser();
   RequestParser(const RequestParser& other);
   RequestParser& operator=(const RequestParser& other);
   ~RequestParser(void);

   std::string                  parseRequestChunk(const char* input, size_t inputLength, bool resetPosition = true);
   size_t                       getAmountParsed(void) const;
   
   void                         setRequest(Request& request);

private:
   static const size_t          MAX_FIELD_LENGTH = 8192;  // To prevent buffer overflow attacks
   static const int             METHOD_MAX_LENGTH = 6;
   static const int             HTTP_VERSION_MAX_LENGTH = 8;
   static const int             URI_MAX_LENGTH = 8000;

   typedef void (RequestParser::*_stateHandler)(char);
   _stateHandler                _parsingStateHandlers[7];
   
   Request*                     request;
   Logger                      log;
   
   //parsing state information
   size_t                       _positionInInput;
   size_t                       _paramterLength;
   char                         _lastChar;
   std::string                  _statusCode;

   //parsing buffers
   std::string                  _headerName;
   std::string                  _headerValue;

   // initialization
   void                         _initializeParsingStateHandlers();

   //parsing
   void                         _parseOneChar(char input);

   //parsing helpers
   bool                         _isHeaderNameChar(char c);
   bool                         _isOWS(char c);
   bool                         _isHeaderValueChar(char c);
   void                         _trimTrailingOWS(std::string& str);

   //header fields parsing
   void                         _changeState(int current, int next);
   void                         _handleStateMethod(char c);
   void                         _handleStateUri(char c);
   void                         _handleStateHttpVersion(char c);
   void	                       _handleStateFieldName(char c);
   void	                       _handleStateOWS(char c);
   void                         _handleStateFieldValue(char c);
   void                         _handleStateFieldPair(char c);

   //header fields parsing helpers
   void                         _storeFieldPair(void);
   void                         _handleHttpMethod(const std::string& input);
   void                         _handleHttpVersion(const std::string& input);
};

#endif /* REQUEST_PARSER_HPP_INCLUDED */
