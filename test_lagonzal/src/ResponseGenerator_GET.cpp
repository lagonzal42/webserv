#include "ResponseGenerator_GET.hpp"
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

/*=============================*/

const char	*ResponseGenerator::generateGetResponse(Parser::Location& currentLoc, Request& req)
{
	if (std::find(currentLoc.methods.begin(), currentLoc.methods.end(), req.getMethod()) == currentLoc.methods.end())
	{
		std::cerr << "Method not allowed" << std::endl;
		//return (ResponseGenerator::errorResponse(METHOD_NOT_ALLOWED, )); //Here I need the full server config, not only the location in order to have access to the error pages
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
		return (ResponseGenerator::errorResponse(NOT_FOUND));
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
		return (ResponseGenerator::errorResponse(FORBIDEN));
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

const char	*ResponseGenerator::getCgiResponse(Request& req)
{
	std::string queryString = req.getQueryString();
	if (!queryString.empty())
	{
		//here we need the envp in order to modify it and add the query string
	}

	int pipes[2];

	if (pipe(pipes) != 0)
	{
		std::cerr << "Failed to create pipes" << std::endl;
		return (ResponseGenerator::errorResponse(500));
	}

	int id = fork();

	if (id == 0)
	{
		dup2(pipes[1], STDOUT_FILENO);
		close(pipes[0]);
		close(pipes[1]);
		std::string fileName = "." + req.getPath();
		char *filepath[] = {const_cast<char *>(fileName.c_str()), NULL};

		if (execve(filepath[0], filepath, ) == -1)
		{
			std::cerr << "Execve failed" << std::endl;
			exit(1);
		}
	}
	else
	{
		int status;
		close(pipes[1]);

		clock_t	start = clock();
		clock_t now = clock() - start;
		while (((float) now) / CLOCKS_PER_SEC < 0.2f)
		{
			std::cout << "Waiting for child process" << std::endl;
		}

		waitpid(id, &status, WNOHANG);
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) != 0)	
				return (ResponseGenerator::errorResponse(404));
			else
			{
				char buffer[BUFFER_SIZE];
				int readed = BUFFER_SIZE;

				std::string content = "";
				while (readed == BUFFER_SIZE)
				{
					readed = read(pipes[0], buffer, BUFFER_SIZE);
					if (readed > 0)
					{
						close(pipes[0]);
						return (ResponseGenerator::errorResponse(INTERNAL_SERVER_ERROR));
					}
					else
						content += std::string(buffer);
				}
				close(pipes[0]);
				//std::cout << "Response: " << content;

				std::stringstream ss;
				ss << content.size();

				std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
				return (response.c_str());
			}
		}
		else //if the child procces has not exited
		{
			kill(id);
			return (ResponseGenerator::errorResponse(408));
		}
	}
}