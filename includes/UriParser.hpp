#ifndef URI_PARSER_HPP_INCLUDED
# define URI_PARSER_HPP_INCLUDED

struct UriComponents;

# include <string>

#include "URL.hpp"
#include "logger/LoggerIncludes.hpp" 

class UriParser
{
	public:
		UriParser();
		UriParser(const UriParser& other);
		UriParser&	operator=(const UriParser& other);
		~UriParser();

		void	setUrl(URL& url);

		bool	parse(const std::string& uriInput);


	private:
		enum States //Dont change order
		{
			STATE_START,
			STATE_SCHEME,
			STATE_DOMAIN,
			STATE_PATH,
			STATE_QUERY,
			STATE_END,
		};

		typedef void (UriParser::*_stateHandler)(char);
   		_stateHandler parsingStateHandlers[5];// number of transition functions

		URL						*_url;
		Logger				log;

		States						_parsingState;
		size_t					_uriIndex;
		const std::string*		_uriInput;

		std::string				_value;

		// initialisation
		void					_initParsingStateHandlers();

		//parsing state handlers
		void 					_changeState(UriParser::States state, UriParser::States nextState);
		void					_handleStateStart(char c);
		void					_handleStateScheme(char c);
		void 					_handleStateDomain(char c);
		void  					_handleStatePath(char c);
		void  					_handleStateQuery(char c);

		//helper functions
		char 					_decodePercentEncodedChar(const std::string& uriInput);
		bool					_isValidSchemeChar(char c);
		bool 					_isValidDomainChar(char c);
		bool					_isValidPathChar(char c);
};

#endif /* URI_PARSER_HPP_INCLUDED */
