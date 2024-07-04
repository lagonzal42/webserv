
#include "Request.hpp"
#include "colors.h"

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include "colors.h"
#include <cstdio>
#include <sys/socket.h>

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 10000
#endif

Request::Request(void)
: _method(""), _version(""), _path(""), _host(""), _port(""), _encoding(""), _queryString(""), _body(""), _contentLength(0), _keepAlive(false)
{}

Request::~Request(void)
{}

Request::Request(const Request& param)
: _method(param.getMethod()), _version(param.getVersion()), _path(param.getPath()), _host(param.getHost()), _port(param.getPort()), _encoding(param.getEncoding()), _queryString(param.getQueryString()), _body(param.getBody()), _contentLength(param.getContentLength()), _keepAlive(param.getConection()) 
{}

int Request::readRequest(int client_socket)
{
	char buffer[BUFFER_SIZE] = {0};
	int valread = BUFFER_SIZE;
	std::string requestStr;

	while (BUFFER_SIZE == valread)
	{
		valread = recv(client_socket, buffer, BUFFER_SIZE, 0);
		if (valread < 0)
		{
			std::cerr << "Error on read" << std::endl;
			close(client_socket);
			return 1;
		}
		else
			requestStr.append(std::string(buffer, valread));
	}
	size_t pos = requestStr.find("\r\n\r\n");
	if (pos != std::string::npos)
		_body.append(requestStr.substr(pos + 4));

	std::istringstream reqStream(requestStr);
	std::string line;
	std::string title;	

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
			_keepAlive = (connection == " keep-alive\r") ? true : false; //the cconnection needs to be kept alive?
		}
		else if (title == "Transfer-Encoding")
		{
			std::string encoding;
			std::getline(line_ss, encoding);
			std::string tempEncoding = encoding;
			size_t posFirst = tempEncoding.find_first_not_of("\r ");
			size_t posLast = tempEncoding.find_last_not_of("\r ");
			_encoding = tempEncoding.substr(posFirst, posLast);
			//break; //As we dont need more info
		}
		else if (title == "Content-Length")
		{
			std::string contentLengthStr;
			std::getline(line_ss, contentLengthStr);
			std::stringstream intss;
			intss << contentLengthStr;
			intss >> _contentLength;
		}
	} //end of while(std::getline)

	while (_body.size() < _contentLength)
	{
		valread = recv(client_socket, buffer, BUFFER_SIZE, 0);
		_body.append(std::string(buffer, valread));
	}
	return 0;
}

std::string	Request::getMethod(void) const {return _method;}
std::string	Request::getQueryString(void) const {return _queryString;}
std::string	Request::getVersion(void) const {return _version;}
std::string	Request::getPath(void) const {return _path;}
std::string	Request::getHost(void) const {return _host;}
std::string	Request::getPort(void) const {return _port;}
std::string	Request::getEncoding(void) const {return _encoding;}
std::string	Request::getBody(void) const {return _body;}
bool		Request::getConection(void) const {return _keepAlive;}
size_t		Request::getContentLength(void) const {return _contentLength;}

void		Request::setKeepAlive(bool pKeepAlive) {_keepAlive = pKeepAlive;}

void	Request::print(void) const
{
	std::cout << "============REQUEST==============" << std::endl;
	std::cout << "Method: \"" << _method << "\"" << std::endl;
	std::cout << "Query String: \"" << _queryString << "\"" << std::endl;
	std::cout << "Version: \"" << _version << "\"" << std::endl;
	std::cout << "Path: \"" << _path << "\"" << std::endl;
	std::cout << "Host: \"" << _host << "\"" << std::endl;
	std::cout << "Port: \"" << _port << "\"" << std::endl;
	std::cout << "Enconding: \\" << _encoding << "|" << std::endl;
	std::cout << "Keep-alive: \"" << _keepAlive << "\"" << std::endl;
	std::cout << "Content-Length: " << _contentLength << std::endl;
	std::cout << "Body length: " << _body.length() << std::endl;
	//getchar();
	//std::cout << "Body: |" << _body << "\\" << std::endl;

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

void	Request::clear()
{
	_method.clear();
	_queryString.clear();
	_version.clear();
	_path.clear();
	_host.clear();
	_port.clear();
	_encoding.clear();
	_keepAlive = 0;
	_contentLength = 0;
	_body.clear();
}

bool	Request::empty(void)
{
	if (_method.empty() && _queryString.empty() && _version.empty() && _path.empty()
		&& _host.empty() && _port.empty() && _encoding.empty() && _encoding.empty()
		&& !_keepAlive && !_contentLength && _body.empty())
		return (true);
	return(false);
}
