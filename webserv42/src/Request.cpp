
#include "Request.hpp"
#include "colors.h"

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 10000
#endif

Request::Request(void)
: _method(""), _version(""), _path(""), _host(""), _port(""), _encoding(""), _queryString(""), _body(""), _keepAlive(false)
{}

Request::~Request(void)
{}

int Request::readRequest(int client_socket)
{
	char buffer[BUFFER_SIZE] = {0};
	int valread = BUFFER_SIZE;
	std::string requestStr = "";
	_method = "";
	_path = "";
	_version = "";

	while (BUFFER_SIZE == valread)
	{
		valread = read(client_socket, buffer, BUFFER_SIZE);
		if (valread < 0)
		{
			std::cerr << "Error on read" << std::endl;
			close(client_socket);
			return 1;
		}
		else
		{
			requestStr += std::string(buffer);
		}
	}

	std::cout << YELLOW << "FULL Request: " << std::endl;
	std::cout << requestStr <<  std::endl;	

	size_t pos = requestStr.find("\r\n\r\n");
	if (pos != std::string::npos)
		_body = requestStr.substr(pos + 4);

	std::istringstream reqStream(requestStr);
	std::string line;
	std::string title;

	std::cout << "_body: " << _body << RESET << std::endl;	

	// separates the request first line and separates it by spaces
	std::getline(reqStream, line);
	std::istringstream line_ss(line);
	line_ss >> _method >> _path >> _version;

	std::istringstream pathfinder(_path);
	std::getline(pathfinder, _path, '?');
	std::getline(pathfinder, _queryString, '?');


	//loops each line apart from the first
	while (std::getline(reqStream, line))
	{
		std::istringstream line_ss(line);

		// divides the line by the first : and saves the first part in title
		std::getline(line_ss, title, ':');
		if (title == "Host")
		{
			std::string hostPort;
			std::getline(line_ss, hostPort);
			size_t colonPos = hostPort.find(':');
			if (colonPos != std::string::npos) // if it can find another : means there is host and port
			{
				_host = hostPort.substr(0, colonPos);
				// I needed to put this function to omid unkown character
				_port = extractNumbers(hostPort.substr(colonPos + 1));
			}
			else //there is only host
				_host = hostPort;
		}
		else if (title == "Connection")
		{
			std::string connection;
			std::getline(line_ss, connection);
			_keepAlive = connection == "keep-alive" ? true : false; //the cconnection needs to be kept alive?
		}
		else if (title == "Transfer-Encoding")
		{
			std::string encoding;
			std::getline(line_ss, encoding);
			_encoding = encoding;
			//break; //As we dont need more info
		}
	} //end of while(std::getline)
	return 0;
}

std::string	Request::getMethod(void) const {return _method;}
std::string	Request::getQueryString(void) const {return _queryString;}
std::string	Request::getVersion(void) const {return _version;}
std::string	Request::getPath(void) const {return _path;}
std::string	Request::getHost(void) const {return _host;}
std::string	Request::getPort(void) const {return _port;}
std::string	Request::getEncoding(void) const {return _encoding;}
//
std::string	Request::getBody(void) const {return _body;}
//
bool		Request::getConection(void) const {return _keepAlive;}


void	Request::print(void) const
{
	std::cout << "============REQUEST==============" << std::endl;
	std::cout << "Method: \"" << _method << "\"" << std::endl;
	std::cout << "Query String: \"" << _queryString << "\"" << std::endl;
	std::cout << "Version: \"" << _version << "\"" << std::endl;
	std::cout << "Path: \"" << _path << "\"" << std::endl;
	std::cout << "Host: \"" << _host << "\"" << std::endl;
	std::cout << "Port: \"" << _port << "\"" << std::endl;
	std::cout << "Enconding: \"" << _encoding << "\"" << std::endl;
	std::cout << "Keep-alive: \"" << _keepAlive << "\"" << std::endl;
	std::cout << "Body: |" << _body << "\\" << std::endl;

}

std::string	Request::extractNumbers(std::string const & str)
{
	std::string ret;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		if (std::isdigit(*it))
			ret.push_back(*it);
	}
	return ret;
}