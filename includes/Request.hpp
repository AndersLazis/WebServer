#ifndef REQUEST_HPP
# define REQUEST_HPP
# include <map>
# include <fstream>
# include <cstring>
# include <string>
# include "interfaces/IData.hpp"
# include "StrContainer.hpp"
# include "Method.hpp"
# include "Data.hpp"
# include "URL.hpp"

#include "logger/LoggerIncludes.hpp"
typedef enum e_chunked_req_state
{
	CH_START,
	CH_SIZE,
	CH_DATA,
	CH_TRAILER,
	CH_COMPLETE,
	CH_ERROR
}			ChunkedReqState;
class Request: virtual public IData
{
	private:
		URL										url;
		Method								method;
		StrContainer						httpVersion;
		ChunkedReqState						chunked_state;
		size_t								remaining_chunk_size;
		std::string							prev_chunk_size;
		Logger								log;
	public:
		std::map<std::string, std::string>	headers;
		ssize_t								content_length;
		size_t								offset;
		size_t								body_start;
		static const size_t					buff_size = 8192;
		char								buff[Request::buff_size];
		Request();
		~Request();
		Request(const Request &other);
		Request	&operator=(const Request &other);
		
		void								setMethod(Method method);
		void								setVersion(StrContainer version);
		void                                setHeader(const std::string& key, const std::string& value);
		void								setUrl(const URL& url);
		void								setDomain(const std::string& domain);
		void								setPort(const std::string& port);
		
		StrContainer						getVersion() const;
		Method								getMethod() const;
		std::map<std::string, std::string>	getHeaders() const;
		URL									getUrl() const;
		ChunkedReqState						getChunkedState() const;
		// Public
		std::string							toString() const;
		void								removeHeader(const std::string& key);
		StringData							save_chunk(std::string output_file);
		bool								isBodyReceived();
};
#endif
