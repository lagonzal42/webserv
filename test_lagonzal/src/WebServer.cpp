#include "WebServer.hpp"

/**
 * @brief	The server loops until a stop signal is received, the 
 * 
 */

void	WebServer::serverLoop(void)
{
	while (!stopSignal)
	{
		int events = poll(pollFDS, pollFDS.size(), 5000);

		if (events != 0)
		{
			for (size_t i = 0; i < pollFDS.size(), i++)
			{
				if (pollFDS[i].revents & POLLIN)
				{
					std::vector<int>::iterator servSockPos = std::find(serverSockets.begin(), serverSockets.end(), pollFDS[i].fd);
					if (servSockPos != serverSockets.end()) //The POLLIN event happened in one of the server sockets
					{
						int vectorPos = servSockPos - serverSockets.begin();
						acceptConnection(vectorPos);
					}
					else
					{
						std::vector<int>::iterator cliSockPos = std::find(clientSockets.begin(), clientSockets.end(), pollFDS[i].fd);
						int vectorPos = cliSockPos - clientSockets.begin();
						readRequest(vectorPos);
						pollFDS[i].events = POLLOUT;
					}
				}
				else if (pollFDS[i].revents & POLLOUT)
				{
					std::vector<int>::iterator cliSockPos = std::find(clientSockets.begin(), clientSockets.end(), pollFDS[i].fd);
					int vectorPos = cliSockPos - clientSockets.begin();
					buildResponse(vectorPos);
					sendResponse(pollFDS[i].fd /*, response here*/);
					cleanVectors(vectorPos);
				}
			} // for (size_t i = 0; i < pollFDS.size(), i++)
		} // if (events != 0)
	} // while (!stopSignal)
}

void	WebServer::acceptConnection(int vectorPos)
{
	struct sockaddr_in	clientAddr;
	socklen_t			clientAddrLen = sizeof(clientAddr);

	int	clientSocket = accept(serverSockets[vectorPos], (struct sockaddr *)&clientAddr, &clientAddrLen);
	pollFDS.push_back({clientSocket, POLLIN, 0});
	clientSockets.push_back(clientSockets);
	requests.push_back(Request());
}

int WebServer::readRequest(int vectorPos)
{
	int result = requests[vectorPos].readRequest(clientSockets[vectorPos]);
	if (result)
	{
		std::cout << "Failed reading the request from the client socket" << std::endl;
		cleanVectors(vectorPos);
	}
}

char	*WebServer::buildResponse(int cliVecPos)
{
	std::string	vec[] = {"GET", "POST", "DELETE"};

	int i = 0;
	for (; i < 3; i++)
	{
		if (requests[cliVecPos].getMethod() == vec[i])
			break;
	}

	switch(i)
	{
		case(GET):

			break;
		case(POST):

			break;

		case(DELETE):

			break;
	}
}

// void WebServer::sendResponse() //still implementing
// {
	
// }


void	cleanVectors(int vectorPos)
{
	int i = 0;
		
	while (clientSockets[vectorPos] != pollFDS[i].fd)
		i++;
	
	pollFDS.erase(pollFDS.begin() + i);
	clientSockets.erase(clientSockets.begin() + vectorPos);
	requests.erase(requests.begin() + vectorPos);
	std::cout << "Closed connection with clients" << std::endl;
}