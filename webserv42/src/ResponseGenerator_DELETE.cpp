#include "ResponseGenerator_DELETE.hpp"
#include <cstdio>   // for remove
#include <algorithm>

//case 1: There is a file to eliminate, and then you could delete it successfully
//case 2: It's not allowed DELETE method
//case 3: It's allowed DELETE method, but failure function METHOD

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

// Could be useful? Or maybe it's already exists
std::string ResponseGeneratorDELETE::generateHttpResponse(const std::string &status, const std::string &title, const std::string &message)
{
	std::ostringstream oss;
	std::string body =
		"<html>"
		"<head><title>" + title + "</title></head>"
		"<body><h1>" + title + "</h1><p>" + message + "</p></body>"
		"</html>";
	oss << "HTTP/1.1 " << status << "\r\n"
		<< "Content-Type: text/html\r\n"
		<< "Content-Length: " << body.size() << "\r\n"
		<< "\r\n" << body;
	return oss.str();
}

std::string ResponseGeneratorDELETE::generateDeleteResponse(Request & req, const Parser::Location & currentLoc, const std::string &fullPath)
{
	std::string response;

	std::cout << "req method: " << req.getMethod() << std::endl;
	std::cout << "location method: " << std::endl;

	for (std::vector<std::string>::const_iterator it = currentLoc.methods.begin(); it != currentLoc.methods.end(); ++it)
		std::cout << *it << " ";
	std::cout << std::endl;

	// Check if the request method is allowed in config
	if (std::find(currentLoc.methods.begin(), currentLoc.methods.end(), req.getMethod()) != currentLoc.methods.end())
	{
		size_t last_slash = fullPath.find_last_of('/');
		if (last_slash != std::string::npos)
		{
			if (remove(fullPath.c_str()) == 0)
			{
				std::cout << CYAN "removed successfully" RESET << std::endl;
				response = generateHttpResponse("200 OK", "File Deleted", "The requested file has been deleted successfully.");
			}
			else
			{
				std::cout << RED "failed to remove file" RESET << std::endl;
				response = generateHttpResponse("500 Internal Server Error", "Internal Server Error", "Failed to delete the requested file on the server.");
			}
		}
		else
		{
			std::cerr << "Bad request" << std::endl;
			response = generateHttpResponse("400 Bad Request", "Bad Request", "Invalid item ID.");
		}
	}
	else
	{
		std::cerr << "Method not allowed" << std::endl;
		response = generateHttpResponse("405 Method Not Allowed", "Method Not Allowed", "The requested Method is not allowed.");
	}
	
	return response;
}
