#include "logger/ConsoleDestination.hpp"

#include <iostream>

void ConsoleDestination::write(const char* message)
{
	std::cerr << message;
	std::cerr.flush();
}
