#include "Utils.hpp"

#include <sstream>

template <typename T>
std::string Utils::toString(const T& value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}