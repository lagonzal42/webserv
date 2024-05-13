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

#define SOCK_N 4


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
