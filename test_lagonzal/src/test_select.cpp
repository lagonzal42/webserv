/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_select.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagonzal <larraingonzalez@gmail.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 10:39:58 by lagonzal          #+#    #+#             */
/*   Updated: 2024/05/09 10:39:58 by lagonzal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_server.hpp"
#include <iostream>
#include <vector>
#include <iterator>
#include <cstring>
#include <fstream>

#ifndef SOCKET_N
#define SOCKET_N 4
#endif


void handle_connection(int client_socket) {
	char buffer[BUFFER_SIZE] = {0};
	int valread = read(client_socket, buffer, BUFFER_SIZE);
	if (valread > 0) {
		std::cout << "Received: " << buffer << std::endl;

		std::ifstream file("../tests_nozomi/docs/index.html", std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Failed to open index.html" << std::endl;
			return;
		}

		std::string html_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		//to_string is not allowed to use in std++98
		//std::itoa(html_content.size()).c_str()
		std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(html_content.size()) + "\r\n\r\n" + html_content;
		std::cout << "response: " << response << std::endl;
		send(client_socket, response.c_str(), response.size(), 0);
	}
	close(client_socket);
}


int main(void)
{
	std::vector<int> serverSockets;
	//std::vector<int> clientSockets;
	std::vector <struct sockaddr_in> serverAddr(SOCKET_N);
	//struct sockaddr_in *serverAdrr = new struct sockaddr_in[SOCKET_N];

	for (size_t i = 0; i < SOCKET_N; i++)
	{
		serverSockets.push_back(socket(AF_INET, SOCK_STREAM, 0));
		
		if (serverSockets[i] == -1)
		{
			perror("server socket creation error");
			return (1);
		}
		
		memset(&serverAddr[i], 0, sizeof(serverAddr[i]));
		serverAddr[i].sin_family = AF_INET;
		serverAddr[i].sin_addr.s_addr = INADDR_ANY;
		serverAddr[i].sin_port = htons(8081 + i);

		if (bind(serverSockets[i], (struct sockaddr *) &serverAddr[i], sizeof(serverAddr[i])) < 0)
		{
			std::cerr << "Bind failed in port " << 8081 + i;
			perror("");
			return (1);
		}
	
		if (listen(serverSockets[i], 2) < 0)
		{
			std::cerr <<  "Failed listen on port " << 8081 + i;
			perror("");
			return (1);
		}
		std::cout << "Listening on port " << 8081 + i << std::endl;
	}

	fd_set readfds;
	FD_ZERO(&readfds);
	
	for (size_t i = 0; i < SOCKET_N; i++)
	{
		FD_SET(serverSockets[i], &readfds);
	}

	size_t maxfd = *(std::max_element(serverSockets.begin(), serverSockets.end()));
	
	std::cout << "SERVER STARTED" << std::endl;
	int a = 0;
	for (int i = 0; i < serverSockets.size(); i++)
		std::cout << "FD in server socket is " << serverSockets[i] << std::endl;
	
	while (true)
	{
		fd_set	readyfds = readfds;

		if (select(maxfd + 1, &readyfds, NULL, NULL, NULL) < 0)
		{
			perror("select error");
			return (1);
		}

		for (size_t i  = 3; i < maxfd + 1; i++)
		{
			std::cout << "lookin if " << i << "IS SET" << std::endl;
			if (FD_ISSET(i, &readyfds))
			{
				std::cout << i << " IS SET, processing request" << std::endl;
				std::vector<int>::iterator it = std::find(serverSockets.begin(), serverSockets.end(), i);
				//std::vector<int>::iterator itC = std::find(clientSockets.begin(), clientSockets.end(), i);
				
				if (it != serverSockets.end() /*&& itC == clientSockets.end()*/)
				{
					struct sockaddr_in clientAddr;
            		socklen_t clientAddrLen = sizeof(clientAddr);
					std::cout << std::endl << std::endl << a++ << std::endl << std::endl;
					std::cout << "NEW CONNECTION" << std::endl;
            		int clientSocket = accept(*it, (struct sockaddr *)&clientAddr, &clientAddrLen);
					//clientSockets.push_back(clientSocket);
					FD_SET(clientSocket, &readfds);
					
					if (clientSocket > maxfd)
                		maxfd = clientSocket;
				}
				else
				{
					std::cout << std::endl << std::endl << "USING OLD CONNECTION" << std::endl << std::endl;
					handle_connection(i);
					FD_CLR(i, &readyfds);
					close(i);
					maxfd--;
				}
			}
		}
	}
}