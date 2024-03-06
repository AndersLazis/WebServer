#ifndef CONSOLEDESTINATION_HPP
#define CONSOLEDESTINATION_HPP

#include "ILogDestination.hpp"

class ConsoleDestination : public ILogDestination
{
	public:
		void write(const char* message);
};

#endif // CONSOLEDESTINATION_HPP
