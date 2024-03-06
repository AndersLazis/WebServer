#include "Manager.hpp"
#include "ConfigParser.hpp"
#include "logger/LoggerIncludes.hpp"

int	main(int argc, char **argv, char **envp)
{
	Logger& log2 = Logger::instance();
	ConsoleDestination console;
	log2.setGeneralLogLevel(LOG_DEBUG);
	log2.addDestination(console);

	Logger log = Logger(Logger::instance(), "main");
	std::string configFilePath = "default.conf";
	if (argc > 2)
	{
		log << LOG_ERROR << "too many arguments\n";
		return (1);
	}
	if (argc == 2)
		configFilePath = argv[1];


	ConfigParser* configParser = NULL;
	int error = 0;
	try
	{
		configParser = new ConfigParser();
		configParser->setEnv(envp);
		configParser->parseConfig(configFilePath);
		// configParser->_printconfig();

		Manager manager(configParser->getServers(), envp);
		delete configParser;
		configParser = NULL;
		manager.startServer();
	}
	catch(const std::exception& e)
	{
		log << LOG_ERROR << e.what() << "\n";
		error = 1;
	}
	log << LOG_INFO << BLUE "shutdown\n" << RESET;
	delete configParser;
	return (error);
}
