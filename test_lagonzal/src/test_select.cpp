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

#include "../inc/test_server.hpp"
#include <iostream>
#include <vector>

#ifndef SOCKET_N
#define SOCKET_N 2
#endif
#ifndef CLIENT_N
#define CLIENT_N SOCKET_N / 2
#endif


int main(void)
{
	std::vector<int> serverSockets;
	int *serverSocket = new int[SOCKET_N];
	struct sockaddr_in *serverAdrr = new struct sockaddr_in[CLIENT_N];

	for (size_t i = 0; i < CLIENT_N; i++)
	{
		serverSockets.push_back(socket(AF_INET, SOCK_STREAM, 0));
		
		if (serverSocket[i] == -1)
		{
			perror("server socket creation error");
			return (1);
		}
		
		memset(&serverAdrr[i], 0, sizeof(serverAdrr[i]));
		serverAdrr[i].sin_familiy = AF_INET
		serverAddr1.sin_addr.s_addr = INADDR_ANY;
		serverAddr1.sin_port = htons(8081 + i);

		if (bind(serverSocket[i], &serverAddr[i], sizeof(serverAddr[i])) < 0)
		{
			std::cerr << "Bind failed in port " << 8081 + i;
			perror();
			return (1);
		}
	
		if (listen(serverSocket[i], 2) < 0)
		{
			std::cerr <<  "Failed listen on port " << 8081 + i;
			perror();
			return (1);
		}

	}

	fd_set readfds;
	FD_ZERO(&readfds);
	
	for (size_t i = 0; i < CLIENT_N; i++)
	{
		FD_SET(serverSocket[i], &readfds);
	}

	int



	


	delete [] serverSocket;
}