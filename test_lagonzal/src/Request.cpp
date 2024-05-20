/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagonzal <larraingonzalez@gmail.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/20 12:50:49 by lagonzal          #+#    #+#             */
/*   Updated: 2024/05/20 12:50:49 by lagonzal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 10000
#endif

Request::Request(int client_socket)
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
        }
        else
        {
            requestStr += std::string(buffer);
        }
    }
    if (valread > 0)
    {
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
                    _port = hostPort.substr(colonPos + 1);
                }
                else //there is only host
                    _host = hostPort;
            }
            else if (title == "Connection")
            {
                std::string connection;
                std::getline(line_ss, connection);
                _keepAlive = connection == "keep-alive" ? true : false; //the connection needs to be kept alive?
                break; //As we dont need more info
            }
        } //end of while(std::getline)
    } // end of if (valread > 0)
}

std::string	Request::getMethod(void) const {return _method;}
std::string	Request::getVersion(void) const {return _version;}
std::string	Request::getPath(void) const {return _path;}
std::string	Request::getHost(void) const {return _host;}
std::string	Request::getPort(void) const {return _port;}
bool		Request::getConection(void) const {return _keepAlive;}