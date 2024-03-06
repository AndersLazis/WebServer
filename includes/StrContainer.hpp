#ifndef STR_CONTAINER_HPP
# define STR_CONTAINER_HPP
# include <string>
# include <algorithm>

class StrContainer: virtual public std::string
{
	public:
		StrContainer();
		StrContainer(std::string base);
		StrContainer(const char *base);
		bool	starts_with(const std::string other) const;
		bool	ends_with(const std::string other) const;
		void	find_first_and_replace(StrContainer old_val, StrContainer new_val);
		void	trim();
};
#endif