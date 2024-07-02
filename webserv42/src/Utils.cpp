#include "Utils.hpp"
#include <sys/stat.h>

#include <sys/types.h>
#include <unistd.h>
#include <string>

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
	int	ret;
	std::cout << "num: " << num << std::endl;

	std::istringstream iss(num);
	if (!(iss >> ret))
	{
		throw std::runtime_error("Failed to convert string to size_t in obtainIntFromStr()");
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



/**
 * @brief Checks if the given path is a file.
 * 
 * @param path The path to check.
 * @return int 1 if the path is a file, 0 otherwise (including if the path does not exist).
 */
int Utils::pathIsFile( const std::string& path )
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return 0; // Path is a directory
		else if (s.st_mode & S_IFREG)
			return 1; // Path is a regular file
		else
			return 0; // Path is neither a regular file nor a directory
	}
	else
	{
		return 0; // stat failed, path likely does not exist
	}
}


// --- Member functions --- //

void debug(std::string str)
{
	std::cout << str << std::endl;
}