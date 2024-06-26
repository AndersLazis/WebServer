#ifndef  REQUESTRECEIVER_HPP
# define  REQUESTRECEIVER_HPP
# include <iomanip>
// Our headers
# include "Method.hpp"
# include "interfaces/IReceiver.hpp"
# include "ResponseSender.hpp"
# include "Request.hpp"
# include "URL.hpp"
# include "Data.hpp"

// -----
#include "RequestParser.hpp"
#include "logger/LoggerIncludes.hpp"

class RequestReceiver: public IReceiver
{
	private:
		int									_fd;
		ReceiverState						state;
		StrContainer						body;
		size_t								_header_pos;
		bool								headersOk;
		StringData							error_code;
		Request								req;
		RequestParser					parser;
		ssize_t								max_body_size;
		ssize_t								received;
		Logger								log;
		std::string							tmp_file;
		bool								finish_request(const std::string& code);
		bool								receive_body();
	public:
		RequestReceiver();
		RequestReceiver(int	fd);
		~RequestReceiver();
		RequestReceiver(const RequestReceiver &other);
		RequestReceiver						&operator=(const RequestReceiver &other);
		
		void								setMaxBodySize(ssize_t maxBodySize);
		
		StrContainer						getBody() const;
		std::string							getTempFile() const;
		Request								&getRequest();
		int									getFd() const;
		// Public
		bool								receive_headers();
		// IReceiver impl
		void								consume();
		ReceiverState						getState() const;
		IData								&produceData();
};
#endif
