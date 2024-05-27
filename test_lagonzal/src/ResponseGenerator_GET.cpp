#include "ResponseGenerator_GET.hpp"
#include <dirent.h>
#include <algorithm>
#include <fstream>
#include <sstream>

const char	*ResponseGenerator::generateGetResponse(Parser::Location& currentLoc, Request& req)
{
	if (std::find(currentLoc.methods.begin(), currentLoc.methods.end(), req.getMethod()) == currentLoc.methods.end())
	{
		std::cerr << "Method not allowed" << std::endl;
		//return (ResponseGenerator::errorResponse(405, )); //Here I need the full server config, not only the location in order to have access to the error pages
	}
	else if (currentLoc.name == "redir")
	{
		std::cout << "Processing Redirection GET request" << std::endl;
		//return (ResponseGenerator::getRedirResponse(??));
	}
	else if (currentLoc.name == "cgi")
	{
		std::cout << "Processing CGI GET request" << std::endl;
		//return (ResponseGenerator::getCgiResponse(req));
	}
	else if (currentLoc.autoindex && !req.getPath().empty() && req.getPath()[req.getPath().length() - 1] == '/')
	{
		std::cout << "Processing autoindex GET request" << std::endl;
		//return (ResponseGenerator::getAutoindexResponse(req));
	}
	else
	{
		std::cout << "Processing file GET request" << std::endl;
		//return (ResponseGenerator::getFileResponse(req));
	}
}

const char	*ResponseGenerator::getFileResponse(Request& req)
{
	std::string response;
	std::stringstream responseStatus;
	std::string filePath = "." + req.getPath();

	responseStatus << 200 << " OK";
	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Failed to open" << filePath << std::endl;
		return (ResponseGenerator::errorResponse(404));
	}
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	std::stringstream ss;
	ss << content.size();

	response = "HTTP/1.1" + responseStatus.str() + "\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
	
	return ((response.c_str()));
}

const char	*ResponseGenerator::getAutoindexResponse(Request& req)
{
	std::string response, responseHead, responseBody;
	std::string dirPath = "." + req.getPath();
	DIR* directory = opendir(dirPath.c_str());
	MimeDict* MimeDict = MimeDict::getMimeDict();
    std::map<std::string, std::string> mime = MimeDict->getMap();
	

	if (directory == NULL)
		return (ResponseGenerator::errorResponse(403));
	responseHead = "HTTP/1.1 200 OK \r\nContent-Type: " + mime[".html"];
	responseBody += "<html><head><title>Index of " + dirPath + "</title></head>";
	responseBody += "<body><h1>Index of " + dirPath + "</h1>";

	responseBody += "<ul>";
	struct dirent	*file = readdir(directory);

	while (file)
	{
		responseBody += "<li><a href=\"" + dirPath + file->d_name + "\"" + file->d_name + "</a></li>";
		file = readdir(directory);
	}
	closedir(directory);
	std::stringstream ss;
	ss << responseBody.size();
	response = responseHead + "\r\nContent-Length: " + ss.str() + "\r\n\r\n" + responseBody;
	return (response.c_str());
}