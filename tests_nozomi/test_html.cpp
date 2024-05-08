#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include "colors.h"

const int BUFFER_SIZE = 1024;

void handle_connection(int client_socket) {
	char buffer[BUFFER_SIZE] = {0};
	int valread = read(client_socket, buffer, BUFFER_SIZE);
	if (valread > 0) {
		std::cout << "Received: " << buffer << std::endl;

		std::ifstream file("docs/index.html", std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Failed to open index.html" << std::endl;
			return;
		}

		std::string html_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		//to_string is not allowed to use in std++98
		std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(html_content.size()) + "\r\n\r\n" + html_content;
		std::cout << CYAN "response: " << response << RESET << std::endl;
		send(client_socket, response.c_str(), response.size(), 0);
	}
	close(client_socket);
}

int main() {
	// Create sockets for both ports
	int serverSocket1 = socket(AF_INET, SOCK_STREAM, 0);
	int serverSocket2 = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket1 == -1 || serverSocket2 == -1) {
		perror("Error creating socket");
		return 1;
	}

	// Set socket options to reuse address
	int yes = 1;
	if (setsockopt(serverSocket1, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1 ||
		setsockopt(serverSocket2, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		perror("Error setting socket options");
		return 1;
	}

	// Bind the sockets to their respective addresses and ports
	struct sockaddr_in serverAddr1, serverAddr2;
	memset(&serverAddr1, 0, sizeof(serverAddr1));
	memset(&serverAddr2, 0, sizeof(serverAddr2));
	serverAddr1.sin_family = AF_INET;
	serverAddr1.sin_addr.s_addr = INADDR_ANY;
	serverAddr1.sin_port = htons(8080);

	serverAddr2.sin_family = AF_INET;
	serverAddr2.sin_addr.s_addr = INADDR_ANY;
	serverAddr2.sin_port = htons(8081);

	if (bind(serverSocket1, (struct sockaddr *)&serverAddr1, sizeof(serverAddr1)) < 0) {
		perror("Bind failed for port 8080");
		return 1;
	}

	if (bind(serverSocket2, (struct sockaddr *)&serverAddr2, sizeof(serverAddr2)) < 0) {
		perror("Bind failed for port 8081");
		return 1;
	}

	// Listen for incoming connections on both ports
	if (listen(serverSocket1, 5) < 0) {
		perror("Listen failed for port 8080");
		return 1;
	}

	if (listen(serverSocket2, 5) < 0) {
		perror("Listen failed for port 8081");
		return 1;
	}

	std::cout << "Server started. Listening on ports 8080 and 8081..." << std::endl;

	// Initialize the set of sockets
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(serverSocket1, &readfds);
	FD_SET(serverSocket2, &readfds);

	int maxfd = std::max(serverSocket1, serverSocket2);

	while (true) {
		// Copy the fd set, since select modifies it
		fd_set tmpfds = readfds;

		// Use select to wait for activity on any of the sockets
		int activity = select(maxfd + 1, &tmpfds, nullptr, nullptr, nullptr);
		if (activity < 0) {
			perror("Select error");
			return 1;
		}

		// If activity on any server socket, accept the connection
		if (FD_ISSET(serverSocket1, &tmpfds))
		{
			int clientSocket;
			struct sockaddr_in clientAddr;
			socklen_t clientAddrLen = sizeof(clientAddr);
			clientSocket = accept(serverSocket1, (struct sockaddr *)&clientAddr, &clientAddrLen);
			if (clientSocket < 0)
			{
				perror("Accept failed");
				return 1;
			}
			std::cout << "Socket1: Connection accepted from " << inet_ntoa(clientAddr.sin_addr) \
			<< ": " << ntohs(clientAddr.sin_port) << std::endl;

			// Handle connection
			handle_connection(clientSocket);
		}
		if (FD_ISSET(serverSocket2, &tmpfds))
		{
			int clientSocket2;
			struct sockaddr_in clientAddr2;
			socklen_t clientAddrLen2 = sizeof(clientAddr2);
			clientSocket2 = accept(serverSocket2, (struct sockaddr *)&clientAddr2, &clientAddrLen2);
			if (clientSocket2 < 0)
			{
				perror("Accept failed");
				return 1;
			}
			std::cout << "Socket2: Connection accepted from " << inet_ntoa(clientAddr2.sin_addr) \
			<< ": " << ntohs(clientAddr2.sin_port) << std::endl;

			// Handle connection
			handle_connection(clientSocket2);
		}
	}

	// Close the server sockets (never reached in this code)
	close(serverSocket1);
	close(serverSocket2);

	return 0;
}
