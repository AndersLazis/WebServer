#ifndef LOGGER_HPP
#define LOGGER_HPP

class ILogDestination;

#include <map>
#include <set>
#include <string>

#include "LogLevel.hpp"

class Logger
{
public:
  static Logger& instance();

  Logger& operator=(const Logger& other);
  Logger(const Logger& other);
  Logger(const Logger& other, std::string logLocationName);
  ~Logger();

  void setGeneralLogLevel(LogLevel level);
  void setLogLevelForDestination(ILogDestination& destination, LogLevel level);
  void setLogLocationName(std::string logLocationName);

  void addDestination(ILogDestination& destination);

  void setTimestampEnabled(bool enabled);

  void log(LogLevel level, const char* message);

  template <typename T>
  friend Logger& operator<<(Logger& logger2, const T& value);
  friend Logger& operator<<(Logger& logger2, LogLevel level);

private:
  bool            timestampEnabled_;
  LogLevel        generalLogLevel_;
  LogLevel        logLevel_;
  std::string     logLocationName_;

  std::set<ILogDestination*>           destinations_;
  std::map<ILogDestination*, LogLevel> destinationLogLevels_;

  Logger();

  LogLevel getGeneralLogLevel() const;
  LogLevel getLogLevelForDestination(ILogDestination& destination) const;
  std::string getTimestamp() const;
};

#include "../../sources/logger/Logger.ipp"

#endif // LOGGER_HPP
