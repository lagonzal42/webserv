#include "ResponseGenerator_GET.hpp"
#include "ResponseGenerator_POST.hpp"
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

std::string ResponseGeneratorPOST::parsePath(std::string servPath, std::string locPath, std::string reqPath)
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

std::string	ResponseGeneratorPOST::generatePostResponse(Request& req, const Parser::Location& currentLoc, const Parser::Server& currentServ, std::vector<char *>& envp)
{
	// Use envp in case of CGI like html form
	debug(RED);
	debug("ResponseGeneratorPOST::generatePostResponse");
	debug("location name:");
	debug(currentLoc.name);
	debug(RESET);

	std::cout << "Gonna generate the clean path with " + currentServ.root + " " + currentLoc.root + " " + req.getPath() << std::endl;
	std::string	cleanPath = ResponseGeneratorPOST::parsePath(currentServ.root, "", req.getPath());
	std::cout << "Clean path is " << cleanPath << std::endl;

	if (std::find(currentLoc.methods.begin(), currentLoc.methods.end(), req.getMethod()) == currentLoc.methods.end())
	{
		std::cerr << MAGENTA << "Method not allowed: " << req.getMethod() << RESET << std::endl;
		return (ResponseGeneratorPOST::errorResponse(METHOD_NOT_ALLOWED, currentServ));
	}
	else if (currentLoc.name == "/upload/")
	{
		std::cout << "Processing upload POST request" << std::endl;
		return (ResponseGeneratorPOST::postResponse(currentLoc, req, envp, currentServ, cleanPath));
	}
	else
	{
		std::cerr << "Location not found" << std::endl;
		return (ResponseGeneratorPOST::errorResponse(NOT_FOUND, currentServ));
	}
}

std::string	ResponseGeneratorPOST::postResponse(const Parser::Location& currentLoc, Request& req, std::vector<char *>& envp, const Parser::Server& currentServ, std::string& cleanPath)
{
	// Extrae la extensión del archivo
    std::string extension = cleanPath.substr(cleanPath.find_last_of("."));

    // Obtén el mapa de MimeDict
    std::map<std::string, std::string> mimeMap = MimeDict::getMimeDict()->getMap();

    // Busca en el mapa la extensión
    std::map<std::string, std::string>::iterator it = mimeMap.find(extension);

	if (it != mimeMap.end()) {
        // La extensión se encontró en el mapa
        std::string mimeType = it->second;

		if (mimeType == "application/x-sh") // check if it's and executable
		{
			return (ResponseGeneratorPOST::postCgiResponse(currentLoc, req, envp, currentServ, cleanPath));
		}
	}
	else if (req.getEncoding() == "chunked") // maybe move it to postCGIresponse
	{
		// this manage chunked uploads
		std::cout << MAGENTA << "Request encode is: " << req.getEncoding() << RESET << std::endl;
		std::cout << "Processing chunked POST request" << std::endl;
		// return (ResponseGeneratorPOST::postChunkedResponse(//no se)); // not done yet
	}
	else
	{
		// Extrae el cuerpo de la solicitud POST
        std::string requestBody = req.getBody();

        // Obtén la ruta de carga de la configuración
        std::string uploadPath = currentLoc.upload_path; // Asume que tienes un método getUploadPath() en la clase Location

        // Combina la ruta de carga con el nombre del archivo
        std::string filePath = uploadPath + "/" + cleanPath;

        // Guarda el cuerpo de la solicitud en un archivo
		std::ofstream ofs(filePath.c_str(), std::ios::binary);
		ofs << requestBody;
		ofs.close();

        std::cout << "Saved file: " << filePath << std::endl;
		
		// std::cerr << "NOT DONE YET" << std::endl;
		// return (ResponseGeneratorPOST::errorResponse(INTERNAL_SERVER_ERROR, currentServ));
	}
	return ("END OF FUNCTION");
}

// check here the data form html form
// Use pipes to send info
std::string	ResponseGeneratorPOST::postCgiResponse(const Parser::Location& currentLoc, Request& req, std::vector<char *>& envp, const Parser::Server& currentServ, std::string& cleanPath)
{
	// Redo, this code is from GET
	if (cleanPath[cleanPath.length() - 1] == '/')
	{
		if (!currentLoc.index.empty())
			cleanPath += currentLoc.index;
	}

	std::string queryString = req.getQueryString();
	if (!queryString.empty())
	{
		//here we need the envp in order to modify it and add the query string
		envp.pop_back();
		std::string qs = "QUERY_STRING=" + req.getQueryString(); 
		envp.push_back(const_cast<char *>(qs.c_str()));
	}

	int pipes[2];

	if (pipe(pipes) != 0)
	{
		std::cerr << "Failed to create pipes" << std::endl;
		return (ResponseGeneratorPOST::errorResponse(INTERNAL_SERVER_ERROR, currentServ));
		//return (NOT_IMPLEMENTED);
	}

	int id = fork();

	if (id == 0)
	{
		close(pipes[0]);
		dup2(pipes[1], STDOUT_FILENO);
		close(pipes[1]);
		std::string binBash = "/bin/bash";
		std::string bash = "bash";
		char *filepath[] = {const_cast<char *>(bash.c_str()), const_cast<char *>(cleanPath.c_str()), NULL};

		if (execve(binBash.c_str(), filepath, &envp[0]) == -1)
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
			//std::cout << "Waiting for child process" << std::endl;
			now = std::time(NULL);
		}

		waitpid(id, &status, WNOHANG);
		if (now - start != 0)
		{
			kill(id, SIGKILL);
			return (ResponseGeneratorPOST::errorResponse(TIMEOUT, currentServ));
		}
		if (WIFEXITED(status))
		{
			//std::cout << "Exited child process" << std::endl;
			if (WEXITSTATUS(status) != 0)
			{
				//std::cout << "Bad exited" << std::endl;
				response = ResponseGeneratorPOST::errorResponse(NOT_FOUND, currentServ);
				//response = NOT_IMPLEMENTED;
			}
			else
			{
				//std::cout << "well exited" << std::endl;
				char buffer[BUFFER_SIZE];
				int readed = BUFFER_SIZE;

				std::string content = "";
				while (readed == BUFFER_SIZE)
				{
					readed = read(pipes[0], buffer, BUFFER_SIZE);
					if (readed < 0)
					{
						//std::cout << "read failed" << std::endl;
						close(pipes[0]);
						response = ResponseGeneratorPOST::errorResponse(INTERNAL_SERVER_ERROR, currentServ);
						return (response);
						//response = NOT_IMPLEMENTED;
					}
					else
					{
						buffer[readed] = '\0';
						content += std::string(buffer);
					}
				}
				close(pipes[0]);
				//std::cout << "Response: " << content;

				std::stringstream ss;
				ss << content.size();

				//std::cout << RED << "content readed from pipe is |" << content << "|" << RESET << std::endl;;

				std::string responseStr = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
				//std::cout << "response str is: " << responseStr << std::endl;
				return(responseStr);
			}
		}
		return (response);
	}
}

std::string ResponseGeneratorPOST::errorResponse(int errorCode, const Parser::Server& currentServ)
{
	std::stringstream	responseStatus;
	std::string			fileName;

	responseStatus << errorCode << " KO";
	fileName = ResponseGeneratorPOST::parsePath(currentServ.root, "", currentServ.error_pages.at(errorCode));

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
	debug("File opened");
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	std::stringstream ss;
	ss << content.size();
	std::string response;

	response = "HTTP/1.1 " + responseStatus.str() + "\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
	
	std::cout << BLUE << response << RESET <<std::endl;

	return ((response));
}