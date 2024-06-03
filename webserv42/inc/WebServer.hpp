/**
 * @file WebServer.hpp
 * @author Larrain Gonzalez (larraingonzalez@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <vector>
#include "Request.hpp"
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <iostream>
#include "Parser.hpp"
#include "Utils.hpp"
#include <cstdlib>
#include <cstring>
#include "ResponseGenerator_GET.hpp"
#include "colors.h"
#include <unistd.h>

#define GET 0
#define POST 1
#define DELETE 2
#define INVALID_METHOD 3

class WebServer
{
	private:
		
		std::vector<int>			serverSockets;	//Servers listening sockets
		std::vector<int>			clientSockets;	//Client sockets to read the request and write the answer
		std::vector<pollfd>			pollFDS;		//pollfd structs to use poll in them and check events
		std::vector<Request>		requests;		//Request class vector in order to process requests and storage them till the socket is ready to write
		std::vector<sockaddr_in>	serverAddrs;	//Server address vector to bind the server sockets
		static bool					stopSignal;		//Stop signal to control if the user has pressed Ctrl + C or Ctrl + Z to stop the server and free memory in case of need
		Parser						config;
		std::vector<char *>			envp;

		// This function will start the signals, no need to be public as it is going to be called from the server class.
		void	startSignals(void);
		static void signalQuitHandle(int);
		static void signalIntHandle(int);
		bool	initializeSockets();
		void	initializeEnvp(char **originalEnvp);
		void	acceptConnection(int servVecPos);
		int		readRequest(int cliVecPos);
		void	processRequest(int vectorPos);
		const char	*buildResponse(int cliVecPos);
		void	sendResponse(int vectorPos, const char *response);
		void	cleanVectors(int vectorPos);
	
	public:

		WebServer(void);
		~WebServer(void);

		
		// Initialize the server based on the what the config file says, probably needs config as parameter
		// We need to agree how the parameters will pass from the config to this init funcition.
		// This funtion will do de socket(), bind(), listen() loop.
		// Return value: if true something went wrong, no server is created
		bool	initialize(char **envp, std::string configFile);
		//This will be the "infinite loop in wich poll is executed"
		void	serverLoop();
		void	serverClose(void);
		void	setStopSignal(bool stop);
	
	public:


	
};

