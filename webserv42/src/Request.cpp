
#include "Request.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include "colors.h"
#include <cstdio>

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 100
#endif

Request::Request(void)
: _method(""), _version(""), _path(""), _host(""), _port(""), _encoding(""), _queryString(""), _body(""), _contentLength(0), _keepAlive(false)
{}

Request::~Request(void)
{}

int Request::readRequest(int client_socket)
{
    char buffer[BUFFER_SIZE] = {0};
    int valread = BUFFER_SIZE;
    std::string requestStr;

    while (BUFFER_SIZE == valread)
    {
        valread = read(client_socket, buffer, BUFFER_SIZE);
        std::cout << GREEN << valread << RESET << std::endl;
        if (valread < 0)
        {
            std::cerr << "Error on read" << std::endl;
            close(client_socket);
            return 1;
        }
        else
        {
            requestStr.append(std::string(buffer, valread));
        }
    }

    std::cout << RED << requestStr << RESET <<  std::endl;

    size_t pos = requestStr.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        std::cout << MAGENTA << requestStr.substr(pos + 4) << RESET << std::endl;
        _body = requestStr.substr(pos + 4);
    }

    std::istringstream reqStream(requestStr);
    std::string line;
    std::string title;

    // Separa la primera línea de la solicitud y la divide por espacios
    std::getline(reqStream, line);
    std::istringstream line_ss(line);
    line_ss >> _method >> _path >> _version;

    std::istringstream pathfinder(_path);
    std::getline(pathfinder, _path, '?');
    std::getline(pathfinder, _queryString, '?');

    // Procesa cada línea aparte de la primera
    while (std::getline(reqStream, line))
    {
        std::istringstream line_ss(line);

        // Divide la línea por el primer ':' y guarda la primera parte en title
        std::getline(line_ss, title, ':');
        if (!title.empty())
        {
            std::string value;
            std::getline(line_ss, value);
            if (!value.empty() && value[0] == ' ') // Elimina el espacio inicial
            {
                value.erase(0, 1);
            }
            // Procesa encabezados específicos
            if (title == "Host")
            {
                size_t colonPos = value.find(':');
                if (colonPos != std::string::npos) // Si encuentra otro ':' significa que hay host y puerto
                {
                    _host = value.substr(0, colonPos);
                    _port = extractNumbers(value.substr(colonPos + 1));
                }
                else // Solo hay host
                    _host = value;
            }
            else if (title == "Connection")
            {
                _keepAlive = value == "keep-alive" ? true : false; // ¿La conexión debe mantenerse viva?
            }
            else if (title == "Transfer-Encoding")
            {
                _encoding = value;
            }
            else if (title == "Content-Length")
            {
                std::stringstream intss;
                intss << value;
                intss >> _contentLength;
            }
        }
    } // Fin del bucle while(std::getline)

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
	std::cout << "Content-Length: " << _contentLength << std::endl;

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
