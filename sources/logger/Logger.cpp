#include "logger/Logger.hpp"

#include <iostream>
#include <ctime>

#include "logger/ILogDestination.hpp"
#include "logger/LogLevel.hpp"

Logger& Logger::instance()
{
	static Logger logger;
	return logger;
}

Logger::Logger()
      : timestampEnabled_(true),
				generalLogLevel_(LOG_UNSET),
				logLevel_(LOG_UNSET),
				logLocationName_("")
{
}

Logger::Logger(const Logger& other)
{
	*this = other;
}

Logger::Logger(const Logger& other, std::string logLocationName)
{
	*this = other;
	this->logLocationName_ = logLocationName;
}

Logger& Logger::operator=(const Logger& other)
{
	if (this != &other)
	{
		this->timestampEnabled_ = other.timestampEnabled_;
		this->generalLogLevel_ = other.generalLogLevel_;
		this->logLevel_ = other.logLevel_;
		this->logLocationName_ = other.logLocationName_;
		this->destinations_ = other.destinations_;
		this->destinationLogLevels_ = other.destinationLogLevels_;
	}
	return *this;
}

Logger::~Logger()
{
}

void Logger::setGeneralLogLevel(LogLevel level)
{
	this->generalLogLevel_ = level;
}

void Logger::setLogLevelForDestination(ILogDestination& destination, LogLevel level)
{
	this->destinationLogLevels_.insert(std::make_pair(&destination, level));
}

void Logger::setLogLocationName(std::string logLocationName)
{
	this->logLocationName_ = logLocationName;
}

void Logger::addDestination(ILogDestination& destination)
{
	this->destinations_.insert(&destination);
}

void Logger::setTimestampEnabled(bool enabled)
{
	this->timestampEnabled_ = enabled;
}

void Logger::log(LogLevel level, const char* message)
{
	std::set<ILogDestination*>::iterator destinationsIt;
	destinationsIt = this->destinations_.begin();
	for (; destinationsIt != this->destinations_.end(); ++destinationsIt)
	{
		LogLevel destinationLogLevel = this->getLogLevelForDestination(**destinationsIt);
		if (level >= destinationLogLevel)
		{
			(*destinationsIt)->write(message);
		}
	}
}

Logger& operator<<(Logger& logger, LogLevel level)
{
	logger.logLevel_ = level;
	std::string message;
	message += "\n" + logger.getTimestamp() + logLevelToString(level) + " [" + logger.logLocationName_ + "]\t\t";
	logger.log(logger.logLevel_, message.c_str());
	return logger;
}

LogLevel Logger::getGeneralLogLevel() const
{
	return this->generalLogLevel_;
}

LogLevel Logger::getLogLevelForDestination(ILogDestination& destination) const
{
	std::map<ILogDestination *, LogLevel>::const_iterator destinationLevelsIt;
	destinationLevelsIt = destinationLogLevels_.find(&destination);;
	if (destinationLevelsIt != destinationLogLevels_.end())
	{
		return destinationLevelsIt->second;
	}
	else
	{
		return this->generalLogLevel_;
	}
}

std::string Logger::getTimestamp() const
{
	char buffer[80];
  std::time_t now = std::time(NULL);
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
	return std::string("[" + std::string(buffer) + "] ");
}
