#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <poll.h>

#define BUFFER_SIZE 1024

void handle_connection(int client_socket)
{
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	int valread = read(client_socket, buffer, BUFFER_SIZE);
	if (valread > 0)
	{
		std::cout << "Received: " << buffer << std::endl;

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
	close(client_socket);
}

int main()
{
	std::vector<int> ports;
	ports.push_back(8080);
	ports.push_back(8081);
	ports.push_back(8089);
	ports.push_back(8090);

	std::vector<int> server_sockets(ports.size());
	// Provide sockets for the number of ports 
	for (size_t i = 0; i < ports.size(); ++i)
	{
		server_sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (server_sockets[i] == -1)
		{
			perror("Error creating socket");
			return 1;
		}

		int yes = 1;
		if (setsockopt(server_sockets[i], SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) == -1)
		{
			perror("Error setting socket options");
			return 1;
		}

		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(ports[i]);

		if (bind(server_sockets[i], reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
		{
			perror("Bind failed for port");
			return 1;
		}

		if (listen(server_sockets[i], 5) < 0)
		{
			perror("Listen failed for port");
			return 1;
		}

		std::cout << "Server started. Listening on port " << ports[i] << "..." << std::endl;
	}

	std::vector<struct pollfd> pollfds(ports.size());
	for (size_t i = 0; i < ports.size(); ++i)
	{
		pollfds[i].fd = server_sockets[i];
		pollfds[i].events = POLLIN;
	}

	// main loop to run the server
	while (true)
	{
		int activity = poll(&pollfds[0], pollfds.size(), -1);
		if (activity < 0)
		{
			perror("Poll error");
			return 1;
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
					return 1;
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

	return 0;
}
