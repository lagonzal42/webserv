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

	std::cout << MAGENTA << req.getMethod() << RESET << std::endl; // delete this line
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
	std::string response;

	if (currentLoc.name == "/cgi/")
		return (ResponseGeneratorPOST::postCgiResponse(currentLoc, req, envp, currentServ, cleanPath));
	if (req.getEncoding() == " chunked\r")
	{
		// this manage chunked uploads
		std::cout << "Processing chunked POST request" << std::endl;
		return (ResponseGeneratorPOST::postChunkedResponse(currentLoc, req, currentServ, cleanPath)); // not done yet
	}
	else
	{
		// Llama a extractFileContent para procesar el cuerpo de la solicitud
    	std::string fileContent = extractFileContent(req.getBody());

		std::cout << MAGENTA << "Content extracted: " << fileContent << RESET << std::endl;

        // Combina la ruta de carga con el nombre del archivo
        std::string filePath = cleanPath + getFilename(req.getBody());

        // Guarda el cuerpo de la solicitud en un archivo
		std::ofstream ofs(filePath.c_str(), std::ios::binary);
		if (!ofs) {
			std::cerr << "Error opening file: " << filePath << std::endl;
		}
		else {
			ofs << fileContent;
			ofs.flush(); // Asegura que todo el contenido se escribe en el archivo
			ofs.close();
			std::cout << "Saved file: " << filePath << std::endl;
		}

		// Construye la cabecera de la respuesta HTTP
		std::stringstream ss;
		ss << fileContent.size();
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + fileContent;
		return (response);
	}
	// curl -X  POST -d "name=DaniPedrosa&age=38" http://localhost:8080/upload/test1 //query string
	// curl -X POST -F "file=@/workspaces/webserv/webserv42/docs/aaa.txt" http://localhost:8080/upload/ // file upload
	// curl -X POST http://localhost:8080/upload/ -H "Content-Type: application/json" -H "Transfer-Encoding: chunked" -d '{"clave": "valor"}' // chunked json
	// curl -X POST http://localhost:8080/upload/ -H "Content-Type: application/json" -H "Transfer-Encoding: chunked" -d '[{"clave": "valor1"}, {"clave": "valor2"}, {"clave": "valor3"}]' // chunked jsons
}

std::string ResponseGeneratorPOST::postChunkedResponse(const Parser::Location& currentLoc, Request& req, const Parser::Server& currentServ, std::string& cleanPath) {
    (void)currentLoc;
    (void)currentServ;
    (void)cleanPath;

    // Respuesta inicial con 100 Continue
    std::string response = "HTTP/1.1 100 Continue\r\n\r\n";

    // Aquí se podría enviar la respuesta 100 Continue
    // sendResponse(response); // Esta es una función hipotética para enviar la respuesta

    // Verifica si el cuerpo de la solicitud está vacío después de recibir 100 Continue
    if ("condicion de continue") {
		"devolver HTTP 100 Continue";
    }
	else {
        // Procesa el cuerpo de la solicitud si no está vacío

        // Obtiene el cuerpo de la solicitud en formato chunked
        std::string chunkedBody = req.getBody();

        // Procesa los chunks para extraer el contenido
        std::string bodyContent = processChunks(chunkedBody);

        // Construye la cabecera de la respuesta final HTTP después de procesar el cuerpo
        std::stringstream ss;
        ss << bodyContent.size();
        response += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + bodyContent;

        return response; // Devuelve la respuesta final después de procesar el cuerpo
    }
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

// Función para extraer el contenido del archivo de una solicitud multipart/form-data
std::string extractFileContent(const std::string& requestBody)
{
    std::string startDelimiter = "filename=\"";
    std::string endDelimiter = "\r\n";
    std::string boundaryStartPattern = "--"; // Identificador del inicio del boundary

    size_t startPos = requestBody.find(startDelimiter);
    if (startPos != std::string::npos) {
        startPos += startDelimiter.length();
        size_t endPos = requestBody.find(endDelimiter, startPos);
        if (endPos != std::string::npos) {
            // Extraer el contenido del archivo
            size_t contentStart = requestBody.find("\r\n\r\n", startPos) + 4; // Saltar hasta el inicio del contenido
            size_t contentEnd = contentStart; // Inicializar contentEnd con contentStart

            // Buscar el inicio del boundary en las siguientes líneas
            size_t nextLinePos = requestBody.find("\r\n", contentStart);
            while (nextLinePos != std::string::npos) {
                size_t boundaryLineStart = requestBody.find(boundaryStartPattern, nextLinePos + 2);
                if (boundaryLineStart == nextLinePos + 2) {
                    // Encontramos el inicio del boundary, ahora retrocedemos para eliminar toda la línea
                    contentEnd = nextLinePos; // Ajustar contentEnd para eliminar la línea del boundary
                    break;
                }
                // Mover al inicio de la siguiente línea
                nextLinePos = requestBody.find("\r\n", nextLinePos + 1);
            }
            return requestBody.substr(contentStart, contentEnd - contentStart);
        }
    }
    return ""; // Devolver una cadena vacía si no se encuentra el contenido
}

// Función para extraer el nombre del archivo de una solicitud multipart/form-data
std::string getFilename(const std::string &filename)
{
    size_t pos = filename.find("filename=");
    if (pos == std::string::npos) {
        return "";
    }
    size_t start = filename.find('"', pos) + 1;
    size_t end = filename.find('"', start);
    return filename.substr(start, end - start);
}

std::string processChunks(const std::string& chunkedData) {
    std::string result;
    std::istringstream stream(chunkedData);
    std::string line;
    while (std::getline(stream, line)) {
        // Elimina el '\r' al final de la línea si está presente
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }

        // Convierte la longitud de hexadecimal a decimal
        unsigned int length = std::strtoul(line.c_str(), NULL, 16);

        // Si la longitud es 0, se alcanzó el final de los chunks
        if (length == 0) {
            break;
        }

        // Lee el contenido del chunk
        std::vector<char> buffer(length + 2); // +2 para el \r\n final
        stream.read(&buffer[0], length + 2);

        // Añade el contenido al resultado, ignorando los últimos dos caracteres (\r\n)
        result.append(buffer.begin(), buffer.end() - 2);
    }
    return result;
}