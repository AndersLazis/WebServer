#ifndef METHOD_HPP
# define METHOD_HPP
# include <string>
# include <stdexcept>
typedef enum e_meth {
	GET,
	POST,
	DELETE,
	PUT,
	PATCH,
	CONNECT,
	OPTIONS,
	HEAD
}			Method;

Method get_method(std::string method);
std::string	getMethodString(Method method);

#endif // METHOD_HPP
