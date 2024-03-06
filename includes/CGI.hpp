#ifndef CGI_HPP
# define CGI_HPP
# include <vector>
# include <unistd.h>
# include "StrContainer.hpp"
# include "Request.hpp"
# include "ResponseSender.hpp"
# include <sys/stat.h>

#include "logger/LoggerIncludes.hpp"

class CGI
{
	private:
		std::vector<std::string>		handler;
		char							**env;
		std::vector<std::string>		paths;
		StrContainer					executablePath;
		StrContainer		    		cgiExt;
		bool							enabled;
		Logger							log;

		// ENV vars
		StrContainer					scriptName;
		StrContainer					scriptFilename;
		StrContainer					pathInfo;
		StrContainer					pathTranslated;
		StrContainer					documentRoot;
		StrContainer					requestURI;
		URL								prevURL;
		StrContainer  					prevExecPath;
		
		StrContainer					checkRegFile(const StrContainer& cgiPath, Request &req);

		std::string 			findExecutablePath(std::vector<std::string> paths,
		                                     std::string handler) const;
		char*							ft_getEnv(char **env) const;
	public:
		void							createEnv(Request &req);
		CGI();
		CGI(const std::vector<std::string>& handler, char **env);
		CGI(const CGI &copy);
		CGI &operator=(const CGI &copy);
		~CGI();
		
		void							setHandler(const std::vector<std::string>& handler);
		void							setEnv(char **envp);
		void							setPaths(const char *path);
		void							setExecutablePath();
		void							setCgiExt(const std::string& ext);
		
		std::vector<std::string>		getHandler();
		char							**getEnv();
		std::string						getExecutablePath(const std::string& full_path);
		char							**getArgs(const std::string& full_path);
		std::string						getCgiExt(void) const;
		URL								getPrevURL(void) const;
		StrContainer getPrevExecPath() const;
		// Public
		void							configure(Request &req, std::string root, std::string index);
		int								execute(Request &req, int *sv, std::string& full_path);
		StrContainer					pathToScript(StrContainer cgiPath, const StrContainer& index, StrContainer filePath, Request &req, StrContainer route_path);
		bool							isEnabled() const;
		void							setupCGI(StrContainer cgiPath, StrContainer script, StrContainer filePath, StrContainer route_path, StrContainer route_root);
};

char									*ft_getEnv(char **env);
#endif
