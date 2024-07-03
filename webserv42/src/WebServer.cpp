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
#include "ResponseGenerator_DELETE.hpp"
#include "ResponseGenerator_POST.hpp"
#include <signal.h>
#ifndef METHOD_NOT_IMPLEMENTED
# define METHOD_NOT_IMPLEMENTED	501
#endif

#ifndef PAYLOAD_TOO_LARGE
# define PAYLOAD_TOO_LARGE 413
#endif

#ifndef HTTP_VERSION_NOT_SUPPORTED
# define HTTP_VERSION_NOT_SUPPORTED 505
#endif




WebServer::WebServer()
{}
WebServer::~WebServer()
{}

bool WebServer::initialize(char **envp, std::string configFile)
{

	startSignals();
	stopSignal = false;
	try
	{
		config.parse(configFile);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Parser exception: " <<e.what() << '\n';
		return (1);
	}

	if (initializeSockets())
		return (1);
	initializeEnvp(envp);
	return (0);
}

bool WebServer::stopSignal = false;

void WebServer::signalIntHandle(int)
{
	stopSignal = true;
	std::cout << "Sigint detected" << std::endl;
}

void WebServer::signalQuitHandle(int)
{
	stopSignal = true;
}

void	WebServer::startSignals(void)
{
	signal(SIGINT, signalIntHandle);
	signal(SIGQUIT, signalQuitHandle);
}

bool	WebServer::initializeSockets(void)
{
	std::map<std::string, Parser::Server>::const_iterator server_iter;
	int yes = 1;
	debug("initializing sockets");
	for (server_iter = config.getServers().begin(); server_iter != config.getServers().end(); ++server_iter)
	{
		const Parser::Server &serv = server_iter->second;
		int port = Utils::obtainIntFromStr(serv.port);
		debug("Initializing port " + serv.port);
		serverSockets.push_back(socket(AF_INET, SOCK_STREAM, 0));
		if (serverSockets.back() == -1)
		{
			std::cerr << "Error creating a socket" << std::endl;
			return (1);
		}
		pollfd newPollFD;
		newPollFD.fd = serverSockets.back();
		newPollFD.events = POLLIN;
		pollFDS.push_back(newPollFD);
	
		if (setsockopt(serverSockets.back(), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) == -1)
		{
			std::cerr << "Error seting socket options" << std::endl;
			return (1);
		}

		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(port);

		if (bind(serverSockets.back(), reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
		{
			std::cerr << "Bind failed" << std::endl;
			return (1);
		}
		if (listen(serverSockets.back(), 5) < 0)
		{
			std::cerr << "Listen failed" << std::endl;
			return (1);
		}
		
	}
	return (0);
}

void	WebServer::initializeEnvp(char **originalEnvp)
{
	int i = 0;

	while (originalEnvp[i])
	{
		envp.push_back(originalEnvp[i]);
		i++;
	}
	envp.push_back(const_cast<char *>(std::string("QUERY_STRING=").c_str()));
	envp.push_back(NULL);
}

void	WebServer::serverLoop(void)
{
	// int counter = 0;
	while (!stopSignal)
	{
		// std::cout << LGREEN "counter: " << counter << RESET << std::endl;
		int events = poll(&pollFDS[0], pollFDS.size(), 5000);
		std::cout << "Polled" << std::endl;
		if (events > 0)
		{
			for (size_t i = 0; i < pollFDS.size(); i++)
			{
				std::cout << "Iterating" << std::endl;
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
						if (requests[vectorPos].empty())
						{
							requests[vectorPos].setKeepAlive(false);
							cleanVectors(vectorPos);
							std::cout << "Empty request" << std::endl;
						}
						else
							pollFDS[i].events = POLLOUT;
					}
				}
				else if (pollFDS[i].revents & POLLOUT)
				{
					//BUILD RESPONSE HERE
					std::vector<int>::iterator cliSockPos = std::find(clientSockets.begin(), clientSockets.end(), pollFDS[i].fd);
					int cliVectorPos = cliSockPos - clientSockets.begin();
					std::string response = buildResponse(cliVectorPos);
					sendResponse(cliVectorPos , response);
					pollFDS[i].events = POLLIN;
					cleanVectors(cliVectorPos);
				}
				std::cout << "End of iteration" << std::endl;
			} // for (size_t i = 0; i < pollFDS.size(), i++)
		} // if (events != 0)
		else if (events == 0)
		{
			std::cout << "No event detected" << std::endl;
		}
		else // Maybe it's necessarry?
		{
			;//poll error
		}
		// counter++;
	} // while (!stopSignal)
}

void	WebServer::acceptConnection(int vectorPos)
{
	struct sockaddr_in	clientAddr;
	socklen_t			clientAddrLen = sizeof(clientAddr);

	int	clientSocket = accept(serverSockets[vectorPos], (struct sockaddr *)&clientAddr, &clientAddrLen);
	
	pollfd newPollFD;
	newPollFD.fd = clientSocket;
	newPollFD.events = POLLIN;
	pollFDS.push_back(newPollFD);
	
	clientSockets.push_back(clientSocket);
	requests.push_back(Request());
}

int WebServer::readRequest(int cliVecPos)
{
	if (requests[cliVecPos].readRequest(clientSockets[cliVecPos]))
	{
		std::cout << "Failed reading the request from the client socket" << std::endl;
		cleanVectors(cliVecPos);
		return(1);
	}
	return(0);
}

std::string WebServer::buildResponse(int cliVecPos)
{
	std::string	vec[] = {"GET", "POST", "DELETE"};

	debug(RED);
	debug("Gonna build reponse in Webserver::buildResponse");
	debug(RESET);

	requests[cliVecPos].print();

	try
	{
		const Parser::Location& currentLoc = config.getCurLocation(requests[cliVecPos].getPath(), requests[cliVecPos].getPort());
		// if (currentLoc.max_body_size != 0  && requests[cliVecPos].getBody().size() > currentLoc.max_body_size)
		// 	return (ResponseGenerator::errorResponse(PAYLOAD_TOO_LARGE, config.getServer(requests[cliVecPos].getPort())));
		// 	return (ResponseGenerator::errorResponse(HTTP_VERSION_NOT_SUPPORTED, config.getServer(requests[cliVecPos].getPort())));
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << e.what() << '\n';
		 /*else*/ if (requests[cliVecPos].getMethod() != "HTTP/1.1")
		 {
			std::cout << "HTTP version not supported" << std::endl;
		 }
	}
	
	int i = 0;
	for (; i < 3; i++)
	{
		if (requests[cliVecPos].getMethod() == vec[i])
			break;
	}
	
	std::string response;
	std::string responseDelete;
	Request& req = requests[cliVecPos];
	req.print();
	std::cout << i << std::endl;
	switch(i)
	{
		case(GET):
			std::cout << "Get Response" << std::endl;
			response = ResponseGenerator::generateGetResponse(req, config.getCurLocation(req.getPath(), req.getPort()), config.getServer(req.getPort()), envp);
			break;
		case(POST):
			std::cout << "Post Response" << std::endl;
			response = ResponseGeneratorPOST::generatePostResponse(req, config.getCurLocation(req.getPath(), req.getPort()), config.getServer(req.getPort()), envp, cliVecPos);
			break;
		case(DELETE):
			std::cout << "Delete Response" << std::endl;
			// static std::string generateDeleteResponse(Request & req, const Parser::Location & currentLoc, Parser::Server & currentSer, const std::string &fullPath);
			response = ResponseGeneratorDELETE::generateDeleteResponse(req, config.getCurLocation(req.getPath(), req.getPort()), config.getServer(req.getPort()));
			break;
		case(INVALID_METHOD):
			std::cout << "Invalid method response" << std::endl;
			response = ResponseGenerator::errorResponse(METHOD_NOT_IMPLEMENTED, config.getServer(req.getPort()));
			break;
	}

	std::cout << "response " << response << std::endl;
	return(response);
}

void WebServer::sendResponse(int vectorPos, std::string& response) //still implementing
{
	if (send(clientSockets[vectorPos], response.c_str(), response.length(), 0) == -1)
		std::cerr << "Send failed" << std::endl;
	else
		std::cout << GREEN "Response sent successfully" RESET << std::endl;
	
}

void	WebServer::cleanVectors(int vectorPos)
{
	int i = 0;

	requests[vectorPos].print();
	while (clientSockets[vectorPos] != pollFDS[i].fd)
		i++;
	if (requests[vectorPos].getConection() == 0)
	{
		//requests[vectorPos].clear();
		close(pollFDS[i].fd);
		pollFDS.erase(pollFDS.begin() + i);
		clientSockets.erase(clientSockets.begin() + vectorPos);
		requests.erase(requests.begin() + vectorPos);
		std::cout << "Closed connection with client" << std::endl;
	}
	else
	{
		std::cout << "Request cleared" << std::endl;
		requests[vectorPos].clear();
	}
}

void	WebServer::serverClose(void)
{
	for (std::vector<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); it++)
		close(*it);
	for (std::vector<int>::iterator it = serverSockets.begin(); it != serverSockets.end(); it++)
		close(*it);
	
	delete	MimeDict::getMimeDict();
}