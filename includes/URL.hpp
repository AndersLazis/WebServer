#ifndef URL_HPP
# define URL_HPP
# include "StrContainer.hpp"
# include <sstream>
# include <iomanip>
# include "interfaces/IData.hpp"
class URL
{
	private:
		StrContainer               domain; // www.google.com
		StrContainer               port; // : 8080
		StrContainer               path; // /search
		StrContainer               query; // ? q=hello
	
	public:
		URL();
		~URL();
		URL(const URL &);
		// Operators
		URL 						&operator=(const URL &);
		URL							operator+(const URL &);
		bool						operator==(const URL &);
		
		void    					setDomain(StrContainer domain);
		void    					setPort(StrContainer port);
		void							setPath(StrContainer path);
		void    					setQuery(StrContainer query);
		
		StrContainer   			getDomain() const;
		StrContainer   			getPort() const;
		StrContainer   			getPath() const;
		StrContainer   			getQuery() const;
		StrContainer					getFullPath() const;
		// Public
		static StrContainer		concatPaths(StrContainer first, StrContainer second);
		static StrContainer		removeFromStart(StrContainer first, StrContainer second);
		static StrContainer		removeFromEnd(StrContainer first, StrContainer second);
		void										addSegment(StrContainer);

};

#endif