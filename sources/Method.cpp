#include "Method.hpp"

Method get_method(std::string method)
{
	if (method == "GET")
		return GET;
	else if (method == "POST")
		return POST;
	else if (method == "DELETE")
		return DELETE;
	else if (method == "PUT")
		return PUT;
	else if (method == "PATCH")
		return PATCH;
	else if (method == "CONNECT")
		return CONNECT;
	else if (method == "OPTIONS")
		return OPTIONS;
	else if (method == "HEAD")
		return HEAD;
	else
		throw std::runtime_error("invalid method\n");
}

std::string getMethodString(Method method)
{
	switch (method)
	{
		case GET: 		return "GET";
		case POST: 		return "POST";
		case DELETE: 	return "DELETE";
		case PUT: 		return "PUT";
		case PATCH: 	return "PATCH";
		case CONNECT: return "CONNECT";
		case OPTIONS: return "OPTIONS";
		case HEAD: 		return "HEAD";
		default: 			return "UNKNOWN";
	}
}
