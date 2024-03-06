#ifndef FILETYPES_HPP
# define FILETYPES_HPP
# include <map>
# include <string>

class FileTypes
{
	private:
		std::map<std::string, std::string>	file_types;
	public:
		FileTypes();
		~FileTypes();
		FileTypes(const FileTypes &other);
		FileTypes	&operator=(const FileTypes &other);
		std::string	getFileType(std::string filename);
		bool		isFileTypeValid(std::string filename);
};

#endif
