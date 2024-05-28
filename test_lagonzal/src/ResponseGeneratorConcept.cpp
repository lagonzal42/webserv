/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseGenerator.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagonzal <larraingonzalez@gmail.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 12:46:31 by lagonzal          #+#    #+#             */
/*   Updated: 2024/05/16 12:46:31 by lagonzal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseGeneratorConcept.hpp"

//Canonic orthodox, not mandatory at this point but is considered a good practice
ResponseGenerator::ResponseGenerator(){}
ResponseGenerator::~ResponseGenerator(){}
ResponseGenerator::ResponseGenerator(const ResponseGenerator&){}
const ResponseGenerator& ResponseGenerator::operator=(const ResponseGenerator& p){return p;}

const char *generateResponse(int responseCode)
{
	std::string response;
	std::string fileName;
	std::stringstream responseStatus;
	switch(responseCode)
	{
		case NOT_FOUND:
			//Check if config has a 404 page path
			//std::string fileName = ServerConfig::getConfig().get404();
			//if (fileName.length() == 0)
			fileName = "../test_vzayas/server/http/default_errors/error404.html";
			responseStatus << responseCode << " KO";
			break;
		case SERVER_ERROR:
			//Check if config has a 404 page path
			//std::string fileName = ServerConfig::getConfig().get404();
			//if (fileName.length() == 0)
			std::string fileName = "../test_vzayas/server/http/default_errors/error500.html";
			responseStatus << responseCode << " KO";
			break;

	}
	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Failed to open" << fileName << std::endl;
		return (generateResponse(NOT_FOUND));
	}
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	std::stringstream ss;
	ss << content.size();

	response = "HTTP/1.1" + responseStatus.str() + "\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
	
	return ((response.c_str()));
}

const char	*generateResponse(int responseCode, int pipeFD)
{
	char		buffer[BUFFERSIZE];
	std::string	content;
	int			readed;
	std::string response;
	
	while ((readed = read(pipeFD, buffer, BUFFERSIZE)) < 0)
	{
		content += std::string(buffer);
	}
	
	close(pipeFD);
	
	if (readed == -1)
		return generateResponse(SERVER_ERROR);
	
	std::stringstream ss;
	ss << content.size();
	std::stringstream responseStatus;
	ss << OK << " OK";

	response = "HTTP/1.1" + responseStatus.str() + "\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
	
	return ((response.c_str()));
}


