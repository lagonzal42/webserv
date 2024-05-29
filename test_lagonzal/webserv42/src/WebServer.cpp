/**
 * @file WebServer.cpp
 * @author Larrain Gonzalez (larraingonzalez@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "WebServer.hpp"
#include "Parser.hpp"
#include <signal.h>
#ifndef METHOD_NOT_IMPLEMENTED
# define METHOD_NOT_IMPLEMENTED	501
#endif

bool WebServer::initialize(char **envp, std::string configFile)
{

	startSignals();

	try
	{
		config.parse(configFile);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Parser exception: " <<e.what() << '\n';
	}

	
	
}

void	WebServer::startSignals(void)
{
	signal(SIGINT, this->signalHandle);
	signal(SIGQUIT, this->signalHandle);
}

void WebServer::signalHandle(void)
{
	stopSignal = true;
}

void	WebServer::serverLoop(void)
{
	while (!stopSignal)
	{
		int events = poll(&pollFDS[0], pollFDS.size(), 5000);

		if (events != 0)
		{
			for (size_t i = 0; i < pollFDS.size(); i++)
			{
				std::vector<int>::iterator servSockPos = std::find(serverSockets.begin(), serverSockets.end(), pollFDS[i].fd);
				if (pollFDS[i].revents & POLLIN)
				{
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
					//BUILD RESPONSE HERE
					std::vector<int>::iterator cliSockPos = std::find(clientSockets.begin(), clientSockets.end(), pollFDS[i].fd);
					int cliVectorPos = cliSockPos - clientSockets.begin();
					char *response = buildResponse(cliVectorPos, servSockPos - serverSockets.begin());
					sendResponse(pollFDS[i].fd , response);
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
	clientSockets.push_back(clientSocket);
	requests.push_back(Request());
}

int WebServer::readRequest(int cliVecPos)
{
	int result = requests[cliVecPos].readRequest(clientSockets[cliVecPos]);
	if (result)
	{
		std::cout << "Failed reading the request from the client socket" << std::endl;
		cleanVectors(cliVecPos);
	}
}

char	*WebServer::buildResponse(int cliVecPos)
{
	std::string	vec[] = {"GET", "POST", "DELETE"};
	Parser::Location currentLoc = config.getCurLocation(requests[cliVecPos].getPath(), requests[cliVecPos].getPort());

	int i = 0;
	for (; i < 3; i++)
	{
		if (requests[cliVecPos].getMethod() == vec[i])
			break;
	}

	// ?? Parser::Server serv = config.getServers()[requests[cliVecPos].getHost()];
	char *response;
	switch(i)
	{
		case(GET):
			std::cout << "Get Resonse" << std::endl;
			response = ResponseGenerator::generateGetResponse(requests[i], currentLoc, config.getServer(requests[cliVecPos].getPort(), requests[cliVecPos].getHost()));
			break;
		case(POST):
			std::cout << "Post Response" << std::endl;
			break;
		case(DELETE):
			std::cout << "Delete Response" << std::endl;
			break;
		case(INVALID_METHOD):
			std::cout << "Invalid method response" << std::endl;
			response = ResponseGenerator::errorResponse(METHOD_NOT_IMPLEMENTED);
			break;
	}
	return(response);
}



// void WebServer::sendResponse() //still implementing
// {
	
// }


void	WebServer::cleanVectors(int vectorPos)
{
	int i = 0;
		
	while (clientSockets[vectorPos] != pollFDS[i].fd)
		i++;
	
	pollFDS.erase(pollFDS.begin() + i);
	clientSockets.erase(clientSockets.begin() + vectorPos);
	requests.erase(requests.begin() + vectorPos);
	std::cout << "Closed connection with client" << std::endl;
}