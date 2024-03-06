#ifndef FILEDESTINATION_HPP
#define FILEDESTINATION_HPP

#include <string>

#include "ILogDestination.hpp"

class FileDestination : public ILogDestination
{
public:
	FileDestination(const std::string& filename);
	FileDestination(const FileDestination& other);
	FileDestination& operator=(const FileDestination& other);
	~FileDestination();

	void write(const char* message);

private:
	std::string filename_;
};

#endif // FILEDESTINATION_HPP
