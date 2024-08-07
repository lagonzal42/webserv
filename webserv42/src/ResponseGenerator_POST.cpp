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

#ifndef CHUNK_SIZE
# define CHUNK_SIZE 1024
#endif

#include "colors.h"
#include "Utils.hpp"


std::string	ResponseGeneratorPOST::generatePostResponse(Request& req, const Parser::Location& currentLoc, const Parser::Server& currentServ, std::vector<char *>& envp)
{
	std::string	cleanPath = ResponseGenerator::parsePath(currentServ.root, "", req.getPath());

	if (std::find(currentLoc.methods.begin(), currentLoc.methods.end(), req.getMethod()) == currentLoc.methods.end())
	{
		std::cerr << MAGENTA << "Method not allowed: " << req.getMethod() << RESET << std::endl;
		return (ResponseGenerator::errorResponse(METHOD_NOT_ALLOWED, currentServ));
	}
	else if (currentLoc.name == "/upload/")
		return (ResponseGeneratorPOST::postResponse(req, cleanPath));
	else if (currentLoc.name == "/cgi/")
		return (ResponseGeneratorPOST::postCgiResponse(currentLoc, req, envp, currentServ, cleanPath));
	else
	{
		std::cerr << "Location not found" << std::endl;
		return (ResponseGenerator::errorResponse(NOT_FOUND, currentServ));
	}
}

std::string	ResponseGeneratorPOST::postResponse(Request& req, std::string& cleanPath)
{
	std::string response;
	
	if (req.getEncoding() == "chunked")
		return (ResponseGeneratorPOST::postChunkedResponse(req));
	else
	{
    	std::string fileContent = extractFileContent(req.getBody());
        std::string filePath = cleanPath + getFilename(req.getBody());

		std::ofstream ofs(filePath.c_str(), std::ios::binary);
		if (!ofs)
			std::cerr << "Error opening file: " << filePath << std::endl;
		else {
			ofs << fileContent;
			ofs.flush();
			ofs.close();
		}
		std::stringstream ss;
		ss << 0;
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n";
		return (response);
	}
}

std::string ResponseGeneratorPOST::postChunkedResponse(Request& req)
{
    std::string response;
    static std::string chunkUnion;
    std::string requestBody = req.getBody();
    bool endChunk = false;

	std::string filename = getFilename(req.getBody());
	if (filename.empty()) {
        std::stringstream ss;
        ss << std::time(0);
        filename = "./docs/upload/chunked.txt";
    }

    while (1)
    {
        std::string chunkSize;
        size_t limitPos = requestBody.find("\r\n");
        std::cout << limitPos << std::endl;
        if (limitPos != std::string::npos)
            chunkSize = requestBody.substr(0, limitPos);
        int decimal = std::strtol(chunkSize.c_str(), NULL, 16);
        if (decimal == 0)
        {
            endChunk = true;
            break;
        }
        else
        {
            chunkUnion.append(requestBody.substr(limitPos + 2, decimal));
            requestBody = requestBody.substr(decimal + limitPos + 4);
        }
    }

    if (!endChunk)
    {
        response = "HTTP/1.1 100 Continue\r\n\r\n";
        req.setKeepAlive(true);
    }
    else
    {
	    std::ofstream temp_file(filename.c_str());
        temp_file << chunkUnion;
	    temp_file.close();
        chunkUnion.clear();
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
    }
    return response;
}

std::string	ResponseGeneratorPOST::postCgiResponse(const Parser::Location& currentLoc, Request& req, std::vector<char *>& envp, const Parser::Server& currentServ, std::string& cleanPath)
{
	std::string response;
	std::string content;
	
	if (cleanPath[cleanPath.length() - 1] == '/')
	{
		if (!currentLoc.index.empty())
			cleanPath += currentLoc.index;
	}

	int pipeToChild[2];
	int pipeToFather[2];

	if (pipe(pipeToFather) != 0 || pipe(pipeToChild) != 0)
	{
		std::cerr << "Failed to create pipes" << std::endl;
		return (ResponseGenerator::errorResponse(INTERNAL_SERVER_ERROR, currentServ));
	}

	int id = fork();

	if (id == 0)
    {
        close(pipeToFather[0]);
        close(pipeToChild[1]);

        dup2(pipeToChild[0], STDIN_FILENO);
        dup2(pipeToFather[1], STDOUT_FILENO);

        std::string post_data = req.getBody();
        std::istringstream iss(post_data);
        std::vector<std::string> words((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

        std::string combined;
        for (std::vector<std::string>::iterator it = words.begin(); it != words.end(); ++it)
        {
            combined += *it;
            if (it + 1 != words.end())
            {
                combined += " ";
            }
        }

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(cleanPath.c_str()));
        argv.push_back(const_cast<char*>(combined.c_str()));
        argv.push_back(NULL);

        if (execve(cleanPath.c_str(), argv.data(), &envp[0]) == -1)
        {
            std::cerr << "Failed to execute CGI program" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    else if (id > 0)
    {
        close(pipeToFather[1]);
        close(pipeToChild[0]);

        std::string post_data = req.getBody();
        write(pipeToChild[1], post_data.c_str(), post_data.size());

		time_t start = std::time(NULL);
		time_t now = std::time(NULL);
		while (now - start < 1)
		{
			now = std::time(NULL);
		}

        char buffer[4096];
        ssize_t count;
		int status;
        waitpid(id, &status, WNOHANG);
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) != 0)
				response = ResponseGenerator::errorResponse(INTERNAL_SERVER_ERROR, currentServ);
			else
			{
				while ((count = read(pipeToFather[0], buffer, sizeof(buffer) - 1)) > 0)
				{
					buffer[count] = '\0';
					content += buffer;
				}
				std::stringstream content_size;
				content_size << content.size();
				response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + content_size.str() + "\r\n\r\n" + content;
				return response;
			}
		}
        else
		{
			std::cerr << "Timeout reached" << std::endl;
			kill(id, SIGKILL);
			return (ResponseGenerator::errorResponse(TIMEOUT, currentServ));
		}
    }
    else
    {
        std::cerr << "Failed to fork" << std::endl;
        return (ResponseGenerator::errorResponse(INTERNAL_SERVER_ERROR, currentServ));
    }
    return response;
}

std::string extractFileContent(const std::string& requestBody)
{
    std::string startDelimiter = "filename=\"";
    std::string endDelimiter = "\r\n";
    std::string boundaryStartPattern = "--";

    size_t startPos = requestBody.find(startDelimiter);
    if (startPos != std::string::npos)
	{
        startPos += startDelimiter.length();
        size_t endPos = requestBody.find(endDelimiter, startPos);

        if (endPos != std::string::npos)
		{
            size_t contentStart = requestBody.find("\r\n\r\n", startPos) + 4;
            size_t contentEnd = contentStart;

            size_t nextLinePos = requestBody.find("\r\n", contentStart);
            while (nextLinePos != std::string::npos)
			{
                size_t boundaryLineStart = requestBody.find(boundaryStartPattern, nextLinePos + 2);
                if (boundaryLineStart == nextLinePos + 2)
				{
                    contentEnd = nextLinePos;
                    break;
                }
                nextLinePos = requestBody.find("\r\n", nextLinePos + 1);
            }
            return requestBody.substr(contentStart, contentEnd - contentStart);
        }
    }
    return "";
}

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

std::string processChunks(const std::string& chunkedData)
{
    std::string result;
    std::istringstream stream(chunkedData);
    std::string line;
    while (std::getline(stream, line))
	{
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        unsigned int length = std::strtoul(line.c_str(), NULL, 16);

        if (length == 0)
            break;

        std::vector<char> buffer(length + 2);
        stream.read(&buffer[0], length + 2);
        result.append(buffer.begin(), buffer.end() - 2);
    }
    return result;
}
