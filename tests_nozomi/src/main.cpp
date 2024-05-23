#include <unistd.h>
#include "Parser.hpp"
#include "colors.h"
#include <stdexcept>
#include <iomanip>

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <poll.h>

#define BUFFER_SIZE 1024

int	obtainIntFromStr(std::string const & num)
{
	int	ret = 0;
	std::istringstream iss(num);
	if (!(iss >> ret))
	{
		throw std::runtime_error("Failed to convert string to size_t in obtainSizeFromStr()");
		// std::cerr << "convert error." << std::endl;
		// return 1; //put exception, end will never get return when it returns error
	}
	return ret;
}

std::map<std::string, std::string> parse_http_request(const std::string& request)
{
	std::map<std::string, std::string> ret;

	// Split the request into lines
	std::istringstream iss(request);
	std::string line;
	while (std::getline(iss, line))
	{
		// Check if the line is empty (end of ret)
		if (line.empty())
		{
			break;
		}

		// Split each line into key-value pairs
		if (line.find("GET") != std::string::npos || line.find("POST") != std::string::npos || line.find("DELETE") != std::string::npos)
		{
			ret["request_line"] = line;
		}
		size_t pos = line.find(':');
		if (pos != std::string::npos)
		{
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			// Trim leading and trailing whitespace from value
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);
			ret[key] = value;
		}
	}

	return ret;
}

void handle_connection(int client_socket)
{
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	int valread = read(client_socket, buffer, BUFFER_SIZE);
	if (valread > 0)
	{
		std::cout << "Received: " << buffer << std::endl;
//TEST PRINT//
		std::cout << "Until hereeee" << std::endl;
//TEST PRINT//

		// Parse the HTTP request to get the requested URL
		// At this moment put here, but it should be in another class
		std::map<std::string, std::string> headers = parse_http_request(buffer);

//TEST PRINT//
		std::cout << "-----**********------" RESET << std::endl;
		std::cout << GREEN "Request print!" RESET << std::endl;
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
			std::cout << GREEN << it->first << ": " RESET << it->second << std::endl;
		std::cout << "-----**********------" RESET << std::endl;

//TEST PRINT//




		// Serve appropriate file based on the requested URL

//*************this is to know how it works HTTP request*************//
        std::string request = buffer;
		if (request.find("8080") != std::string::npos) {
			std::ifstream file("docs/index.html", std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "Failed to open index.html" << std::endl;
				return;
			}

			std::string html_content;
			char ch;
			while (file.get(ch))
			{
				html_content += ch;
			}
			file.close();

			char content_length[20]; // Assuming content length won't exceed 20 characters
			sprintf(content_length, "Content-Length: %lu", html_content.size());
			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
			response += content_length;
			response += "\r\n\r\n" + html_content;
			std::cout << "response: " << response << std::endl;
			send(client_socket, response.c_str(), response.size(), 0);
		}    
		else if (request.find("8081") != std::string::npos) {
			std::ifstream file("docs/indexcolor.html", std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "Failed to open index.html" << std::endl;
				return;
			}

			std::string html_content;
			char ch;
			while (file.get(ch))
			{
				html_content += ch;
			}
			file.close();

			char content_length[20]; // Assuming content length won't exceed 20 characters
			sprintf(content_length, "Content-Length: %lu", html_content.size());
			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
			response += content_length;
			response += "\r\n\r\n" + html_content;
			std::cout << "response: " << response << std::endl;
			send(client_socket, response.c_str(), response.size(), 0);
		}
//*************this is to know how it works HTTP request*************//
		else
		{
		// Example:
		std::string requested_url = "/"; // Place holder for requested URL
		if (requested_url == "/") {
			std::ifstream file("docs/indexbird.html", std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "Failed to open index.html" << std::endl;
				return;
			}

			std::string html_content;
			char ch;
			while (file.get(ch))
			{
				html_content += ch;
			}
			file.close();

			char content_length[20]; // Assuming content length won't exceed 20 characters
			sprintf(content_length, "Content-Length: %lu", html_content.size());
			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
			response += content_length;
			response += "\r\n\r\n" + html_content;
			std::cout << "response: " << response << std::endl;
			send(client_socket, response.c_str(), response.size(), 0);
		}
		}
	}
	close(client_socket);
}



std::vector<int>	setSockets(Parser const & parser)
{
	std::map<std::string, Parser::Server>::const_iterator server_iter;
	std::vector<int> ret(parser.getServers().size());
	int	i = 0;
	for (server_iter = parser.getServers().begin(); server_iter != parser.getServers().end(); ++server_iter)
	{
		const Parser::Server &server = server_iter->second;
		int	port = obtainIntFromStr(server.port);
		ret[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (ret[i] == -1)
		{
			perror("Error creating socket");
			// return NULL;
		}

		int yes = 1;
		if (setsockopt(ret[i], SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) == -1)
		{
			perror("Error setting socket options");
			// return 1;
		}

		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(port);

		if (bind(ret[i], reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
		{
			perror("Bind failed for port");
			// return 1;
		}

		if (listen(ret[i], 5) < 0)
		{
			perror("Listen failed for port");
			// return 1;
		}
		i++;
		std::cout << "Server started. Listening on port " << port << "..." << std::endl;
	}
	return ret;
}

void	mainLoop(std::vector<struct pollfd> pollfds, std::vector<int> server_sockets)
{
	while (true)
	{
		int activity = poll(&pollfds[0], pollfds.size(), -1);
		if (activity < 0)
		{
			perror("Poll error");
			// return 1;
		}

		for (size_t i = 0; i < pollfds.size(); ++i)
		{
			if (pollfds[i].revents & POLLIN)
			{
				int client_socket;
				struct sockaddr_in client_addr;
				socklen_t client_addr_len = sizeof(client_addr);
				client_socket = accept(server_sockets[i], reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
				if (client_socket < 0)
				{
					perror("Accept failed");
					// return 1;
				}
				std::cout << "Socket " << i+1 << ": Connection accepted from " << inet_ntoa(client_addr.sin_addr)
							<< ": " << ntohs(client_addr.sin_port) << std::endl;

				handle_connection(client_socket);
			}
		}
	}
	for (size_t i = 0; i < server_sockets.size(); ++i)
	{
		close(server_sockets[i]);
	}
}

int	main(int argc, char *argv[])
{
//************** config parser **************//
	std::string	config_path;
	if (argc == 2)
		config_path = argv[1];
	else if (argc == 1)
		config_path = "configurations/default.conf";
	else
	{
		std::cerr << "Error: too many arguments." << std::endl;
		return 1;
	}
	Parser parser;//construct without argv, but with webserv.conf
	// Parser parser(config_path);//constructor with config_path

	try
	{
		parser.parse(config_path);
	}
	catch (const std::exception & e)
	{
		std::cerr << "Parser error: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "\n" << std::setw(40) << "======>>> print test <<<======" << std::endl;
	std::cout << parser << RESET;
	std::cout << std::setw(40) << "======>>> print test <<<======" << "\n" << std::endl;
//************** config parser **************//

//************** check if it's working the getter **************//
parser.getCurLocation("/var/www/html/aaa", "8080");


//************** check if it's working the getter **************//

	
	//maybe I need to put Server class outside of the parser class

//************** provide sockets **************//
	std::vector<int> server_sockets = setSockets(parser);
//************** provide sockets **************//

	std::vector<struct pollfd> pollfds(server_sockets.size());
	for (size_t i = 0; i < server_sockets.size(); ++i)
	{
		pollfds[i].fd = server_sockets[i];
		pollfds[i].events = POLLIN;
	}

	mainLoop(pollfds, server_sockets);

	for (size_t i = 0; i < server_sockets.size(); ++i)
	{
		close(server_sockets[i]);
	}
	return 0;
}
