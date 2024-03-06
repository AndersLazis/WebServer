#ifndef UTILS_HPP
#define UTILS_HPP

# include <string>

class Utils {
public:
	template <typename T>
	static std::string toString(const T& value);
};

#include "../sources/Utils.ipp"

#endif // UTILS_HPP
