/**
 * @file Webserv.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-24
 * 
 * @copyright Copyright (c) 2024
 * 
 * 
 * I forgot to push last changes from home so in order to not create a conflict i will work in this file.
 */


#include "WebServer.hpp"

void    WebServer::processRequest(int vectorPos)
{
	std::string methods[] = {"GET", "POST", "DELETE", NULL};

	int i = 0;
	for (; i < 3; i++)
	{
		if (methods[i] == requests[vectorPos].getMethod())
			break;
	}

	switch (i)
	{
		case GET:
				return(processGetRequest(requests[i]));
			break;
		
		case POST:
			/* code */
			break;
		
		case DELETE:
			/* code */
			break;

		case INVALID_METHOD:
			/* code */
			break;
	}
}