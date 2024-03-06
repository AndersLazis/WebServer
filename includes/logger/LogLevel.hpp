#ifndef LOGLEVEL_HPP
#define LOGLEVEL_HPP

#include <string>

enum LogLevel
{
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_UNSET
};

std::string logLevelToString(LogLevel level);

#endif // LOGLEVEL_HPP
