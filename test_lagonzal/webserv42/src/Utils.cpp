#include "Utils.hpp"

// --- Functions for Orthodox canonical class form --- //

Utils::Utils( void ){}
Utils::~Utils( void ){}
Utils::Utils( Utils const & src )
{
	if (this != &src)
	{
		this->operator=(src);
	}
}
Utils & Utils::operator=( Utils const & src )
{
	if (this != &src)
	{
		this->~Utils();
		new(this) Utils(src);
	}
	return *this;
}
// --- Functions for Orthodox canonical class form --- //

// --- Member functions --- //
std::string	Utils::extractNumbers(std::string const & str)
{
	std::string ret;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		if (std::isdigit(*it))
			ret.push_back(*it);
	}
	return ret;
}
// --- Member functions --- //

