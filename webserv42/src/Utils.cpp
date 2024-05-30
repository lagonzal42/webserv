#include "Utils.hpp"
#include <sys/stat.h>

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

int	Utils::obtainIntFromStr(std::string const & num)
{
	int	ret = 0;
	std::istringstream iss(num);
	if (!(iss >> ret))
	{
		throw std::runtime_error("Failed to convert string to size_t in obtainSizeFromStr()");
		// std::cerr << "convert error." << std::endl;
		// return 1; //put exception, end will never get return when it returns error
	}
	return ret;
}


bool Utils::fileExists(const std::string &path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}
// --- Member functions --- //

void debug(std::string str)
{
	std::cout << str << std::endl;
}