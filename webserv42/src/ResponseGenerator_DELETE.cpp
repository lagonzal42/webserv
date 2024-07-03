#include "ResponseGenerator_DELETE.hpp"
#include "ResponseGenerator_GET.hpp"
#include "Utils.hpp"
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
	// static std::string exHeader;
	std::string	fullPath = ResponseGenerator::parsePath(currentSer.root, "", req.getPath());

// test print
	std::cout << YELLOW "fullPath: " << fullPath << RESET << std::endl;
	std::cout << "req method: " << req.getMethod() << std::endl;
	std::cout << "location method: ";
// test print

	for (std::vector<std::string>::const_iterator it = currentLoc.methods.begin(); it != currentLoc.methods.end(); ++it)
		std::cout << *it << " ";
	std::cout << std::endl;

	// Check if the request method is allowed in config
	if (std::find(currentLoc.methods.begin(), currentLoc.methods.end(), req.getMethod()) != currentLoc.methods.end())
	{
		// Check if the path is for a file
		if (Utils::pathIsFile(fullPath) == 1)
		{
			if (remove(fullPath.c_str()) == 0)
			{
				std::cout << CYAN "removed successfully" RESET << std::endl;
				response = generateHttpResponse("200 OK", "File Deleted", "The requested file has been deleted successfully.");
				// response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 20";
				// response.append("\r\n\r\n");
				// response.append("Removed successfully\n");
			}
			else
			{
				std::cout << RED "failed to remove file: forbiden" RESET << std::endl;
				response = ResponseGenerator::errorResponse(FORBIDEN, currentSer);
			}
		}
		else
		{
			std::cout << RED "failed to remove file: not found" RESET << std::endl;
			response = ResponseGenerator::errorResponse(NOT_FOUND, currentSer);
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
