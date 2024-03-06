#ifndef HTTP_CODES_HPP
# define HTTP_CODES_HPP

# include <string>

class HttpCodes
{
public:
	static std::string	getCompleteStatus(const std::string& code);
	static std::string	getReasonPhrase(const std::string& code);

private:
	HttpCodes();
	HttpCodes(const HttpCodes& other);
	HttpCodes &operator=(const HttpCodes& other);
	~HttpCodes();

	static std::string	getReasonPhraseByStatusCode(const std::string& statusCode);
	static std::string	getReasonPhraseFor1xxCodes(int statusCode);
	static std::string	getReasonPhraseFor2xxCodes(int statusCode);
	static std::string	getReasonPhraseFor3xxCodes(int statusCode);
	static std::string	getReasonPhraseFor4xxCodes(int statusCode);
	static std::string	getReasonPhraseFor5xxCodes(int statusCode);
};

#endif // HTTP_CODES_HPP
