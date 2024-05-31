#include "ResponseGenerator_DELETE.hpp"
#include <cstdio>   // for remove
#include <algorithm>



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

// const char	* ResponseGeneratorDELETE::generateDeleteResponse(Request & req, const Parser::Location & currentLoc, const std::string &fullPath)
std::string ResponseGeneratorDELETE::generateDeleteResponse(Request & req, const Parser::Location & currentLoc, const std::string &fullPath)
{
	std::ostringstream oss;
	std::string response;
	std::string body;

	std::cout << "req method: " << req.getMethod() << std::endl;
	std::cout << "location method: " << std::endl;
	for (std::vector<std::string>::const_iterator it = currentLoc.methods.begin(); it != currentLoc.methods.end(); ++it)
		std::cout << *it << " ";
	std::cout << std::endl;

	if (std::find(currentLoc.methods.begin(), currentLoc.methods.end(), req.getMethod()) == currentLoc.methods.end())
	{
		std::cerr << "Method not allowed" << std::endl;
		return NULL;//(ResponseGenerator::errorResponse(METHOD_NOT_ALLOWED, currentServ)); //Here I need the full server config, not only the location in order to have access to the error pages
	}
	if (remove(fullPath.c_str()) == 0)
	{
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
	}
	else
	{
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
	return response;
	// return response.c_str();
}
