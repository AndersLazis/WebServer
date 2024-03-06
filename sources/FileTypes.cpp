/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileTypes.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmenke <cmenke@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/15 20:00:08 by aputiev           #+#    #+#             */
/*   Updated: 2024/03/05 16:00:40 by cmenke           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileTypes.hpp"

FileTypes::FileTypes()
{
	this->file_types["html"] =	"text/html";
	this->file_types["htm"] =	"text/html";
	this->file_types["shtml"] =	"text/html";
	this->file_types["css"] =	"text/css";
	this->file_types["xml"] =	"text/xml";
	this->file_types["c"] =		"text/x-c";
	this->file_types["cpp"] =	"text/x-c++";
	this->file_types["cs"] =	"text/x-csharp";
	this->file_types["gif"]	= 	"image/gif";
	this->file_types["jpeg"] =	"image/jpeg";
	this->file_types["jpg"] =	"image/jpeg";
	this->file_types["txt"] =	"text/plain";
	this->file_types["png"]	=	"image/png";
	this->file_types["ico"] =	"image/x-icon";
	this->file_types["bmp"] =	"image/x-ms-bmp";
	this->file_types["mp4"] =	"video/mp4";
	this->file_types["mpeg"] =	"video/mpeg";
	this->file_types["mpg"] =	"video/mpeg";
	this->file_types["avi"] =	"video/x-msvideo";
	this->file_types["json"] =	"application/json";
	this->file_types["doc"] =	"application/msword";
	this->file_types["pdf"] =	"application/pdf";
	this->file_types["bin"] =	"application/octet-stream";
	this->file_types["exe"] =	"application/octet-stream";
	this->file_types["img"] =	"application/octet-stream";	
	this->file_types["bin"] =	"application/octet-stream";
	this->file_types["pl"] =	"application/x-perl";
	this->file_types["py"] =	"text/x-python";
	this->file_types["sh"] =	"application/x-sh";
	this->file_types["avi"] =	"video/x-msvideo";

}

FileTypes::~FileTypes() {}

FileTypes::FileTypes(const FileTypes &other)
{
	*this = other;
}

FileTypes &FileTypes::operator=(const FileTypes &other)
{
	if (this != &other) {
		this->file_types = other.file_types;
	}
	return (*this);
}

bool FileTypes::isFileTypeValid(std::string filename)
{
	std::string extension;
	size_t dot_pos;

	dot_pos = filename.find_last_of(".");
	if (dot_pos != std::string::npos)
		extension = filename.substr(dot_pos + 1);
	else
		return (false);

	if (extension != "c" && extension != "cpp" && extension != "cs" && extension != "py" && extension != "sh" 
			&& extension != "php" && extension != "pl" && extension != "bin" && extension != "avi" && extension != "cgi")
		return (true);

	return (false);
}


std::string FileTypes::getFileType(std::string filename)
{
	std::string extension;
	size_t dot_pos;

	dot_pos = filename.find_last_of(".");
	if (dot_pos != std::string::npos)
		extension = filename.substr(dot_pos + 1);
	else
		return ("application/octet-stream");
	return (this->file_types[extension]);
}