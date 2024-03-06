#include "logger/Logger.hpp"

#include <sstream>

template <typename T>
Logger& operator<<(Logger& logger, const T& value)
{
  std::stringstream ss;
  ss << value;
  logger.log(logger.logLevel_, ss.str().c_str());
  return logger;
}
