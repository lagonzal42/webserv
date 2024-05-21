/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagonzal <larraingonzalez@gmail.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/21 11:47:43 by lagonzal          #+#    #+#             */
/*   Updated: 2024/05/21 11:47:43 by lagonzal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

void	WebServer::serverLoop(void)
{
	while (!stopSignal)
	{
		int events = poll(pollFDS, pollFDS.size(), 500);

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
						std::vector<int>::iterator cliSockPos = std::find(clientSockets.begin(), clientSockets.end(), pollFDS.fd);
						int vectorPos = cliSockPos - clientSockets.begin();
						processRequest(vectorPos);
						pollFDS[i].events = POLLOUT;
					}
				}
				else if (pollFDS[i].revents & POLLOUT)
				{
					//BUILD RESPONSE HERE
					std::vector<int>::iterator cliSockPos = std::find(clientSockets.begin(), clientSockets.end(), pollFDS.fd);
					int vectorPos = cliSockPos - clientSockets.begin();
					sendResponse(pollFDS[i].fd /*, response here*/);
					cleanVectors(vectorPos);
				}
			} // for (size_t i = 0; i < pollFDS.size(), i++)
		} // if (events != 0)
	} // while (!stopSignal)
}