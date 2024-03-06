#include "logger/LogLevel.hpp"

std::string logLevelToString(LogLevel level)
{
	std::string levelStr;
	levelStr.reserve(9);
	levelStr = "[";
	switch (level)
	{
		case LOG_DEBUG:
			levelStr += "DEBUG";
			break;
		case LOG_INFO:
			levelStr += "INFO";
			break;
		case LOG_WARN:
			levelStr += "WARN";
			break;
		case LOG_ERROR:
			levelStr += "ERROR";
			break;
		default:
			levelStr += "UNSET";
	}
	levelStr += "]";
	return levelStr;
}
