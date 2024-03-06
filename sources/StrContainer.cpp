#include "StrContainer.hpp"

StrContainer::StrContainer(): std::string() {}

StrContainer::StrContainer(std::string base): std::string(base) {}

StrContainer::StrContainer(const char *base): std::string(base) {}

bool StrContainer::starts_with(const std::string other) const
{
	return (std::search(this->begin(), this->end(), other.begin(), other.end()) == this->begin());
}

bool StrContainer::ends_with(const std::string other) const
{
	return (std::find_end(this->begin(), this->end(), other.begin(), other.end()) == (this->end() - other.length()));
}

void StrContainer::find_first_and_replace(StrContainer old_val, StrContainer new_val)
{
	size_t	pos;
	size_t	delta = 0;
	if (!old_val.size())
		return ;
	pos = this->find(old_val, delta);
	if (pos != this->npos)
		*this = this->replace(pos, old_val.size(), new_val, 0, new_val.size());
}

void StrContainer::trim()
{
	if (!this->size())
		return ;
	size_t	pos = 0;
	while (pos < this->size() && std::isspace((*this)[pos]))
		pos++;
	if (pos > 0)
		this->erase(0, pos);
	pos = 0;
	while (this->size() > 0 && std::isspace((*this)[this->size() - 1]))
		this->erase(this->size() - 1, 1);
}
