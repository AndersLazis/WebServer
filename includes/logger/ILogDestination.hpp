#ifndef ILOGDESTINATION_HPP
#define ILOGDESTINATION_HPP

class ILogDestination
{
	public:
  	virtual void write(const char* message) = 0;
};

#endif // ILOGDESTINATION_HPP