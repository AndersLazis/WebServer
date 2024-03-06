/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Data.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmenke <cmenke@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 19:40:14 by aputiev           #+#    #+#             */
/*   Updated: 2024/03/05 16:00:40 by cmenke           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Data.hpp"

StringData::StringData(): std::string(), type(D_ERROR) {}

StringData::StringData(std::string base): std::string(base), type(D_ERROR) {}

StringData::StringData(std::string base, DataType type): std::string(base), type(type) {}

StringData::StringData(const char *base): std::string(base), type(D_ERROR) {}

StringData::StringData(const char *base, DataType type): std::string(base), type(type) {}

StringData::~StringData() {}

DataType StringData::getType() const
{
	return this->type;
}

bool StringData::operator==(StringData &second)
{
	return static_cast<const std::string&>(*this) == static_cast<const std::string&>(second);
}

bool StringData::operator==(std::string &string)
{
	return static_cast<const std::string&>(*this) == string;
}
