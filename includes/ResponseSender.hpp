#ifndef RESPONSESENDER_HPP
# define RESPONSESENDER_HPP
// Cpp libs
# include <sstream>
# include <fstream>
# include <typeinfo>
// C libs
# include <cerrno>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <ctime>
// .h headers
# include <netdb.h>
# include <unistd.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/socket.h>
// Our headers
# include "FileTypes.hpp"
# include "Data.hpp"
# include "interfaces/ISender.hpp"
# include "HttpCodes.hpp"
# include "Request.hpp"
# include "StrContainer.hpp"

#include "logger/LoggerIncludes.hpp"

# ifdef __linux__
#  define SEND_FLAGS MSG_NOSIGNAL
# else
#  define SEND_FLAGS 0
# endif
class ResponseSender: public ISender
{
	private:
		std::string							httpVersion;
		StringData							statusCode;
		std::string							reason;
		std::map<std::string, std::string>	headers;
		std::map<int, std::string>			error_pages;
		StrContainer						body;
		std::string							_plain;
		size_t								body_size;
		size_t								sent;
		std::string							file;
		std::ifstream						*file_stream;
		int									fd;
		bool								ready;
		bool								_finished;
		bool								cgi;
		bool								plain_sent;
		Logger								log;
		void								_build();
	public:
		ResponseSender();
		ResponseSender(int fd);
		~ResponseSender();
		ResponseSender(const ResponseSender &other);
		ResponseSender	operator=(const ResponseSender &other);
		
		void		setBody(std::string body);
		void		setHeader(std::string key, std::string value);
		void		setStatusCode(std::string code);
		void		setReason(std::string reason);
		void		setContentTypes(std::string filename);
		void		setFd(int fd);
		void		setErrorPages(std::map<int, std::string> map);
		void		setFile(std::string file);
		
		std::string getBody() const;
		int			getFd() const;
		// Public
		void		build_file(const std::string& filename, bool custom_error=false);
		void		build_error(const std::string& status_code, bool custom_error=false);
		void		build_dir_listing(const std::string& content);
		void		build_redirect(const std::string& redirect);
		void		build_cgi_response(const std::string& response);
		bool		run();
		bool		_send();
		// ISender impl
		bool		readyToSend();
		void		setData(IData &data);
		bool		finished();
		void		sendData();
};
#endif
