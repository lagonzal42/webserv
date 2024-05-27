#include "Request.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <algorithm>

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 10000
#endif

Request::Request(void)
: _method(""), _version(""), _path(""), _host(""), _port(""), _encoding("")
{}

Request::~Request(void)
{}

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

int Request::readRequest(int client_socket)
{
	char buffer[BUFFER_SIZE] = {0};
	int valread = BUFFER_SIZE;
	std::string requestStr = "";

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
	std::istringstream reqStream(requestStr);
	std::string line;
	std::string title;


	// separates the request first line and separates it by spaces
	std::getline(reqStream, line);
	std::istringstream line_ss(line);
	line_ss >> _method >> _path >> _version;

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
				// std::replace(_host.begin(), _host.end(), '\r', '\0');
				// std::replace(_port.begin(), _port.end(), '\r', '\0');
			}
			else //there is only host
				_host = hostPort;
		}
		else if (title == "Connection")
		{
			std::string connection;
			std::getline(line_ss, connection);
			_keepAlive = connection == "keep-alive" ? true : false; //the connection needs to be kept alive?
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
std::string	Request::getVersion(void) const {return _version;}
std::string	Request::getPath(void) const {return _path;}
std::string	Request::getHost(void) const {return _host;}
std::string	Request::getPort(void) const {return _port;}
std::string	Request::getEncoding(void) const {return _encoding;}
bool		Request::getConection(void) const {return _keepAlive;}