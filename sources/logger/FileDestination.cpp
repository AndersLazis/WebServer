#include "logger/FileDestination.hpp"

#include <iostream>
#include <fstream>

FileDestination::FileDestination(const std::string& filename)
	: filename_(filename)
{
}

FileDestination::FileDestination(const FileDestination& other)
{
	*this = other;
}

FileDestination& FileDestination::operator=(const FileDestination& other)
{
	this->filename_ = other.filename_;
	return *this;
}

FileDestination::~FileDestination() {}

void FileDestination::write(const char* message)
{
	std::ofstream file(this->filename_.c_str(), std::ios::app);
	file << message;
	if (file.fail())
	{
		std::cerr << "Failed to write to log file " << this->filename_ << std::endl;
	}
	else
		file.flush();
	file.close();
}
