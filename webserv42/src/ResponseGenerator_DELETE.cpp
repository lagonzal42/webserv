#include "ResponseGenerator_DELETE.hpp"
#include "ResponseGenerator_GET.hpp"
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

std::string ResponseGeneratorDELETE::generateDeleteResponse(Request & req, const Parser::Location & currentLoc, const Parser::Server & currentSer)
{
	std::string response;
	std::string	fullPath = ResponseGenerator::parsePath(currentSer.root, "", req.getPath());

// test print
	std::cout << YELLOW "fullPath: " << fullPath << RESET << std::endl;
	std::cout << "req method: " << req.getMethod() << std::endl;
	std::cout << "location method: ";

	for (std::vector<std::string>::const_iterator it = currentLoc.methods.begin(); it != currentLoc.methods.end(); ++it)
		std::cout << *it << " ";
	std::cout << std::endl;
// test print

	// Check if the request method is allowed in config
	if (std::find(currentLoc.methods.begin(), currentLoc.methods.end(), req.getMethod()) != currentLoc.methods.end())
	{
		size_t last_slash = fullPath.find_last_of('/');
		if (last_slash != std::string::npos)
		{
			std::cout << GREEN "Try to remove: " << fullPath << RESET << std::endl;
			if (remove(fullPath.c_str()) == 0)
			{
				std::cout << CYAN "removed successfully" RESET << std::endl;
				response = generateHttpResponse("200 OK", "File Deleted", "The requested file has been deleted successfully.");
			}
			else
			{
				std::cout << RED "failed to remove file" RESET << std::endl;
				response = ResponseGenerator::errorResponse(INTERNAL_SERVER_ERROR, currentSer);
			}
		}
		else
		{
			std::cerr << RED "Bad request" RESET << std::endl;
			response = ResponseGenerator::errorResponse(BAD_REQUEST, currentSer);
		}
	}
	else
	{
		std::cerr << RED "Method not allowed" RESET << std::endl;
		response = ResponseGenerator::errorResponse(METHOD_NOT_ALLOWED, currentSer);
	}
	std::cout << response << std::endl;
	return response;
}
