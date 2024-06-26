#ifndef  CGIRECEIVER_HPP
# define  CGIRECEIVER_HPP
# include <netdb.h>
# include <unistd.h>
# include <sstream>
# include <cerrno>
# include <string.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/socket.h>
// Our headers
# include "Method.hpp"
# include "interfaces/IReceiver.hpp"
# include "Data.hpp"

#include "logger/LoggerIncludes.hpp"

class CGIReceiver: public IReceiver
{
	private:
		int						fd;
		ReceiverState			state;
		StringData				data;
		Logger					log;
		std::string				headers;

	public:
		CGIReceiver();
		CGIReceiver(int	fd);
		~CGIReceiver();
		CGIReceiver(const CGIReceiver &other);
		CGIReceiver				&operator=(const CGIReceiver &other);
		
		int						getFd() const;
		// IReceiver impl
		void					consume();
		ReceiverState			getState() const;
		IData					&produceData();
};
#endif
