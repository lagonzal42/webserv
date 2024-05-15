/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_poll.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagonzal <larraingonzalez@gmail.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 17:02:31 by lagonzal          #+#    #+#             */
/*   Updated: 2024/05/09 17:02:31 by lagonzal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_server.hpp"
#include <vector>
#include <poll.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

#define SOCK_N 4
#ifndef NOT_FOUND
# define NOT_FOUND 404
#endif

extern char ** environ;

void	doCgi(std::string& fileName, int client_socket)
{
	int pipes[2];


	if (pipe(pipes) != 0)
	{
		std::cout << "Failed to create pipes" << std::endl;
		return ;
	}

	int id = fork();

	if (id == 0)
	{
		dup2(pipes[1], STDOUT_FILENO);
		close(pipes[1]);
		close(pipes[0]);
		const char *filepath = {fileName.c_str()};
		char *mutableFilepath = const_cast<char*>(filepath);

		if (execve(filepath, &mutableFilepath, environ) == -1)
		{
			std::cout << "execve failed" << std::endl;
			std::string content;
			std::cout << "Response: " << content;

			std::stringstream ss;
    		ss << content.size();

			std::string response = "HTTP/1.1 404 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
			send(client_socket, response.c_str(), response.size(), 0);
			close(client_socket);
			exit(0);
		}
	}
	else
	{
		close(pipes[1]);
		
		waitpid(id, NULL, 0);
		char buffer[BUFFER_SIZE];
		int readed = read(pipes[0], buffer, BUFFER_SIZE);

		close(pipes[0]);
		std::string content(buffer);
		if (readed > 0)
		{
			std::cout << "Response: " << content;

			std::stringstream ss;
    		ss << content.size();

			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
			send(client_socket, response.c_str(), response.size(), 0);
		}
	}
}

void	doStandard(std::string& fileName, int client_socket)
{
	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Failed to open index.html" << std::endl;
		return;
	}

	std::string html_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	//to_string is not allowed to use in std++98
	//std::itoa(html_content.size()).c_str()

	std::stringstream ss;
    ss << html_content.size();

	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + html_content;
	std::cout << "response: " << response << std::endl;
	send(client_socket, response.c_str(), response.size(), 0);
}


void handle_connection(int client_socket) {
	char buffer[BUFFER_SIZE] = {0};
	int valread = read(client_socket, buffer, BUFFER_SIZE);
	if (valread > 0) {
		std::cout << "Received: " << buffer << std::endl;

		std::string req(buffer);
		std::string method;
		std::istringstream reqStream(req);

		std::getline(reqStream, method, ' ');
		
		std::cout << "Method: " << method << std::endl;

		std::string fileName;
		std::getline(reqStream, fileName, ' ');
		fileName = "." + fileName;

		std::cout << "Requested file: " << fileName << std::endl;

		if (fileName.find("/cgi-bin") != std::string::npos)
		{
			std::cout << "The file is in the cgi-bin directory" << std::endl;
			doCgi(fileName, client_socket);
		}
		else
		{
			std::cout << "The file is not in the cgi-bin directory" << std::endl;
			doStandard(fileName, client_socket);
		}
		// std::ifstream file("../tests_nozomi/docs/index.html", std::ios::in | std::ios::binary);
		// if (!file.is_open()) {
		// 	std::cerr << "Failed to open index.html" << std::endl;
		// 	return;
		// }

		// std::string html_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		// file.close();

		// //to_string is not allowed to use in std++98
		// //std::itoa(html_content.size()).c_str()
		// std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(html_content.size()) + "\r\n\r\n" + html_content;
		// std::cout << "response: " << response << std::endl;
		// send(client_socket, response.c_str(), response.size(), 0);
	}
	close(client_socket);
}

int main(void)
{
	std::vector<int> serverSockets;
	std::vector<pollfd> fds;
	std::vector<sockaddr_in> serverAddrs(SOCK_N);

	for (size_t i = 0; i < SOCK_N; i++)
	{
		int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

		if (serverSocket == -1)
		{
			perror("Server socket creation error");
			return (1);
		}

		//struct sockaddr_in serverAddr;
		memset(&serverAddrs[i], 0, sizeof(serverAddrs[i]));
		serverAddrs[i].sin_family = AF_INET;
		serverAddrs[i].sin_port = htons(8080 + i);
		serverAddrs[i].sin_addr.s_addr = INADDR_ANY;

		if (bind(serverSocket, (struct sockaddr *) &serverAddrs[i], sizeof(serverAddrs[i])) < 0)
		{
			std::cerr << "Bind failed in port" << 8080 + i;
			perror("");
			return (1);
		}
		if (listen(serverSocket, 5) < 0)
		{
			std::cerr <<  "Failed listen on port " << 8080 + i;
			perror("");
			return (1);
		}

   		std::cout << "Listening on port " << 8081 + i << std::endl;

		fds.push_back({serverSocket, POLLIN, 0});
		serverSockets.push_back(serverSocket);
	}


	while (true)
	{
		poll(&fds[0], fds.size(), -1);

		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN) //POLLIN flag means that the socket is ready. revets is the event that is returned by poll
			{
				std::vector<int>::iterator it = std::find(serverSockets.begin(), serverSockets.end(), fds[i].fd);
				if (it != serverSockets.end()) //Creates a client socket
				{
					struct sockaddr_in clientAddr;
					socklen_t clientAddrLen = sizeof(clientAddr);
					int clientSocket = accept(*it , (struct sockaddr *)&clientAddr, &clientAddrLen);
					fds.push_back({clientSocket, POLLIN, 0});
				}
				else //Uses the client socket and closes it
				{
					handle_connection(fds[i].fd);
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					i--;
				}
			}
		}
		std::cout << "FDS in fds vector are: " << std::endl;
		for (size_t i = 0; i < fds.size(); i++)
		{
			std::cout << fds[i].fd << std::endl;
		}
		std::cout << std::endl;
	}


}
