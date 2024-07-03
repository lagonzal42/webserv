#include "ResponseGenerator_GET.hpp"
#include "ResponseGenerator_DELETE.hpp"
#include <dirent.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <sys/wait.h>

/*=========HTTP CODES=========*/

#ifndef OK
# define OK 200
#endif
#ifndef NO_CONTENT
# define NO_CONTENT	204
#endif
#ifndef PERMANENT_REDIRECT
# define PERMANENT_REDIRECT	308
#endif
#ifndef FORBIDEN
# define FORBIDEN 403
#endif
#ifndef NOT_FOUND
# define NOT_FOUND 404
#endif
#ifndef METHOD_NOT_ALLOWED
# define METHOD_NOT_ALLOWED 405
#endif
#ifndef TIMEOUT
# define TIMEOUT 408
#endif
#ifndef INTERNAL_SERVER_ERROR
# define INTERNAL_SERVER_ERROR 500
#endif

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1024
#endif

#ifndef NOT_IMPLEMENTED
# define NOT_IMPLEMENTED "Not implemented ^_^"
#endif

#include "colors.h"
#include "Utils.hpp"

/*=============================*/

std::string ResponseGenerator::parsePath(std::string servPath, std::string locPath, std::string reqPath)
{
	bool lastSlash;
	bool firstSlash;

	if (reqPath[reqPath.length() - 1] != '/')
		lastSlash = false;
	else
		lastSlash = true;
	if (!servPath.empty() && servPath[0] == '/')
		firstSlash = true;
	else
		firstSlash = false;

	std::istringstream			pathSS(servPath + locPath + reqPath);
	std::string					pathPart;
	std::vector<std::string>	pathPartsVec;

	while (std::getline(pathSS, pathPart, '/'))
	{
		if (!pathPart.empty())
			pathPartsVec.push_back(pathPart);
	}


	std::cout << BLUE;
	for (size_t i = 0; i < pathPartsVec.size(); i++)
		std::cout << pathPartsVec[i] << std::endl;
	
	std::cout << RESET;

	std::vector<std::string> cleanPathVec;
	for (std::vector<std::string>::iterator it = pathPartsVec.begin(); it != pathPartsVec.end(); it++)
	{
		if (*it == "..")
		{
			if (!cleanPathVec.empty())
				cleanPathVec.pop_back();
		}
		else if (*it != "." || it == pathPartsVec.begin())
			cleanPathVec.push_back(*it);
	}

	std::string cleanPath;

	if (firstSlash)
		cleanPath += "/";

	while (!cleanPathVec.empty())
	{
		cleanPath += cleanPathVec.front();
		if (cleanPathVec.size() > 1 || lastSlash)
			cleanPath += "/";
		cleanPathVec.erase(cleanPathVec.begin());
	}
	return (cleanPath);
}

std::string	ResponseGenerator::generateGetResponse(Request& req, const Parser::Location& currentLoc, const Parser::Server& currentServ, std::vector<char *>& envp)
{
	debug(RED);
	debug("ResponseGenerator::generateGetResponse");
	debug("location name:");
	debug(currentLoc.name);
	debug(RESET);

	std::cout << "Gonna generate the clean path with " + currentServ.root + " " + currentLoc.root + " " + req.getPath() << std::endl;
	std::string	cleanPath = ResponseGenerator::parsePath(currentServ.root, "", req.getPath());
	std::cout << "Clean path is " << cleanPath << std::endl;

	if (std::find(currentLoc.methods.begin(), currentLoc.methods.end(), req.getMethod()) == currentLoc.methods.end())
	{
		std::cerr << "Method not allowed" << std::endl;
		return (ResponseGenerator::errorResponse(METHOD_NOT_ALLOWED, currentServ));
	}
	else if (currentLoc.name == "/redir/")
	{
		std::cout << "Processing Redirection GET request" << std::endl;
		return (ResponseGenerator::getRedirResponse(currentLoc.redirect));
	}
	else if (currentLoc.name == "/cgi/")
	{
		std::cout << "Processing CGI GET request" << std::endl;
		return (ResponseGenerator::getCgiResponse(currentLoc, req, envp, currentServ, cleanPath));
	}
	else if (currentLoc.autoindex && req.getPath()[req.getPath().length() - 1] == '/')
	{
		std::cout << "Processing autoindex GET request" << std::endl;
		return (ResponseGenerator::getAutoindexResponse(currentServ, cleanPath));
	}
	else
	{
		std::cout << "Processing file GET request" << std::endl;
		return (ResponseGenerator::getFileResponse(currentLoc, currentServ, cleanPath));
	}
}

std::string ResponseGenerator::getFileResponse(const Parser::Location& currentLoc, const Parser::Server& currentServ, std::string& cleanPath)
{
	std::string response;
	std::stringstream responseStatus;
	std::string content;

	if (cleanPath[cleanPath.length() - 1] == '/')
	{
		if (!currentLoc.index.empty())
			cleanPath += currentLoc.index;
		else
			cleanPath += "index.html";

	}
	
	responseStatus << 200 << " OK";
	std::cout << "cleanPath is " << cleanPath << std::endl;
	
	try
	{
		std::ifstream file(cleanPath.c_str(), std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			std::cerr << "Failed to open" << cleanPath << std::endl;
			return (ResponseGenerator::errorResponse(NOT_FOUND, currentServ));
		}
		content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
	}
	catch (std::exception& e)
	{
		return (ResponseGenerator::errorResponse(500, currentServ));
	}
	
	std::stringstream ss;
	ss << content.size();

	size_t extensionPoint = cleanPath.find_last_of('.');
	std::string contentType;
	if (extensionPoint != std::string::npos)
	{
		std::string extension = cleanPath.substr(extensionPoint);
		MimeDict* MimeDict = MimeDict::getMimeDict();
		std::map<std::string, std::string> mime = MimeDict->getMap();
		std::cout << "extesion is " << extension << std::endl;
		try
		{
			contentType = mime[extension];
		}
		catch(const std::exception& e)
		{
			contentType = "text/plain";
		}
		
	}

	response = "HTTP/1.1 " + responseStatus.str() + "\r\nContent-Type: " + contentType + "\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
	
	std::cout << BLUE << response << RESET <<std::endl;

	return ((response));
}

std::string	ResponseGenerator::getRedirResponse(const std::string& redir)
{
	std::string response;
	std::stringstream responseStatus;

	responseStatus << PERMANENT_REDIRECT << " OK";
	response = "HTTP/1.1 " + responseStatus.str() + "\r\nContent-Type: text/html\r\nContent-Length: 0" + "\r\nLocation: " + redir + "\r\n\r\n";

	return (response);
}

std::string	ResponseGenerator::getAutoindexResponse(const Parser::Server& currentServ, std::string& cleanPath)
{
	std::string response, responseHead, responseBody;
	DIR* directory = opendir(cleanPath.c_str());
	MimeDict* MimeDict = MimeDict::getMimeDict();
    std::map<std::string, std::string> mime = MimeDict->getMap();

	if (directory == NULL)
		return (ResponseGenerator::errorResponse(FORBIDEN, currentServ));
	responseHead = "HTTP/1.1 200 OK \r\nContent-Type: " + mime[".html"];
	responseBody += "<html><head><title>Index of " + cleanPath + "</title></head>";
	responseBody += "<body><h1>Index of " + cleanPath + "</h1>";

	responseBody += "<ul>";
	struct dirent	*file = readdir(directory);

	while (file)
	{
		std::cout << GREEN << "file into directory is " << file->d_name << RESET << std::endl;
		std::string fileName = std::string(file->d_name);
		if (file->d_type)
		responseBody += "\n<li><a href=\"" + fileName + "\">" + fileName + "</a></li>";
		file = readdir(directory);
	}
	closedir(directory);
	std::stringstream ss;
	ss << responseBody.size();
	response = responseHead + "\r\nContent-Length: " + ss.str() + "\r\n\r\n" + responseBody;
	return (response);
}

std::string	ResponseGenerator::getCgiResponse(const Parser::Location& currentLoc, Request& req, std::vector<char *>& envp, const Parser::Server& currentServ, std::string& cleanPath)
{
	if (cleanPath[cleanPath.length() - 1] == '/')
	{
		if (!currentLoc.index.empty())
			cleanPath += currentLoc.index;
	}

	std::string queryString = req.getQueryString();
	if (!queryString.empty())
	{
		envp.pop_back();
		std::string qs = "QUERY_STRING=" + req.getQueryString(); 
		envp.push_back(const_cast<char *>(qs.c_str()));
	}

	int pipes[2];

	if (pipe(pipes) != 0)
	{
		std::cerr << "Failed to create pipes" << std::endl;
		return (ResponseGenerator::errorResponse(INTERNAL_SERVER_ERROR, currentServ));
	}

	int id = fork();

	if (id == 0)
	{
		std::string executor, program;

		close(pipes[0]);
		dup2(pipes[1], STDOUT_FILENO);
		close(pipes[1]);

		if (Utils::getExtension(cleanPath) == ".py")
		{
			executor = "/usr/bin/python3";
			program = "python3";
		}
		else
		{
			executor = "/bin/bash";
			program = "bash";
		}


		char *filepath[] = {const_cast<char *>(program.c_str()), const_cast<char *>(cleanPath.c_str()), NULL};

		if (execve(executor.c_str(), filepath, &envp[0]) == -1)
		{
			std::cerr << "Execve failed" << std::endl;
			exit(1);
		}
		exit(0);
	}
	else
	{
		close(pipes[1]);
		std::string response;
		int status;

		time_t start = std::time(NULL);
		time_t now = std::time(NULL);
		while (now - start < 1)
		{
			now = std::time(NULL);
		}
		
		waitpid(id, &status, WNOHANG);
		if (WIFEXITED(status)) //child exited
		{
			std::cout << RED << "Child exited" << RESET << std::endl;
			if (WEXITSTATUS(status) != 0) //error exit status
				response = ResponseGenerator::errorResponse(INTERNAL_SERVER_ERROR, currentServ);
			else //normal exit status (0)
			{
				char buffer[BUFFER_SIZE];
				int readed = BUFFER_SIZE;

				std::string content = "";
				while (readed == BUFFER_SIZE)
				{
					readed = read(pipes[0], buffer, BUFFER_SIZE);
					if (readed < 0)
					{
						close(pipes[0]);
						response = ResponseGenerator::errorResponse(INTERNAL_SERVER_ERROR, currentServ);
						return (response);
					}
					else
					{
						buffer[readed] = '\0';
						content += std::string(buffer);
					}
				}
				close(pipes[0]);

				std::stringstream ss;
				ss << content.size();
				std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
				return(response);
			}
		}
		else
		{
			std::cout << "Timeout reached" << std::endl;
			kill(id, SIGKILL);
			return (ResponseGenerator::errorResponse(TIMEOUT, currentServ));
		}
		return (response);
	}
}

std::string ResponseGenerator::errorResponse(int errorCode, const Parser::Server& currentServ)
{
	std::stringstream	responseStatus;
	std::string			fileName;

	responseStatus << errorCode << " KO";
	try
	{
		fileName = ResponseGenerator::parsePath(currentServ.root, "", currentServ.error_pages.at(errorCode));
	}
	catch (std::exception& e)
	{
		fileName = "";
	}

	std::ifstream file(fileName.c_str());
	if (!file.is_open())
	{
		std::cerr << "Failed to open " << fileName << std::endl;
		if (errorCode != INTERNAL_SERVER_ERROR)
		{
			return (errorResponse(INTERNAL_SERVER_ERROR, currentServ));
		}
		else
		{
			std::string response;
			response = "HTTP/1.1 500 OK\r\nContent-Type: text/html\r\nContent-Length: 12";
			response.append("\r\n\r\n");
			response.append("Server Error\n");
			return (response);
		}
	}
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	std::stringstream ss;
	ss << content.size();
	std::string response;

	response = "HTTP/1.1 " + responseStatus.str() + "\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
	
	return ((response));
}