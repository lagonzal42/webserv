#include "ResponseGenerator_DELETE.hpp"
#include <cstdio>   // for remove


ResponseGeneratorDELETE::ResponseGeneratorDELETE( void ){}
ResponseGeneratorDELETE::~ResponseGeneratorDELETE( void ){}
ResponseGeneratorDELETE::ResponseGeneratorDELETE( ResponseGeneratorDELETE const & src )
{
	if (this != &src)
	{
		this->operator=(src);
	}
}
ResponseGeneratorDELETE & ResponseGeneratorDELETE::operator=( ResponseGeneratorDELETE const & src )
{
	if (this != &src)
	{
		this->~ResponseGeneratorDELETE();
		new(this) ResponseGeneratorDELETE(src);
	}
	return *this;
}

const char	* ResponseGeneratorDELETE::generateDeleteResponse(const std::string &fullPath)
{
	std::ostringstream oss;
	std::string response;
	std::string body;

	if (remove(fullPath.c_str()) == 0) {
		std::cout << CYAN "removed successfully" RESET << std::endl;
		body =
			"<html>"
			"<head><title>File Deleted</title></head>"
			"<body><h1>File Deleted</h1><p>The requested file has been deleted successfully.</p></body>"
			"</html>";
		oss << "HTTP/1.1 200 OK\r\n"
			<< "Content-Type: text/html\r\n"
			<< "Content-Length: " << body.size() << "\r\n"
			<< "\r\n" << body;
	} else {
		std::cout << RED "failed to remove file" RESET << std::endl;
		body =
			"<html>"
			"<head><title>Internal Server Error</title></head>"
			"<body><h1>Internal Server Error</h1><p>Failed to delete the requested file on the server.</p></body>"
			"</html>";
		oss << "HTTP/1.1 500 Internal Server Error\r\n"
			<< "Content-Type: text/html\r\n"
			<< "Content-Length: " << body.size() << "\r\n"
			<< "\r\n" << body;
	}

	response = oss.str();
	return response.c_str();
}
