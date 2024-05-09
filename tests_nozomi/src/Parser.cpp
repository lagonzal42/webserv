#include <fstream>
#include "Parser.hpp"
#include "colors.h"
#include <algorithm>

// --- Functions for Orthodox canonical class form --- //

Parser::Parser( void ): _conf_file("configurations/default.conf")
{
	std::cout << _conf_file << ": Called default constructor!" << std::endl;

}
Parser::~Parser( void )
{
	std::cout << _conf_file << ": Called destructor, bye!" << std::endl;
}
Parser::Parser( Parser const & src )
{
	if (this != &src)
	{
		this->_conf_file = src._conf_file;
	}
}
Parser & Parser::operator=( Parser const & src )
{
	if (this != &src)
	{
		this->~Parser();
		new(this) Parser(src);
	}
	return *this;
}
// --- Functions for Orthodox canonical class form --- //


// --- Member functions --- //

Parser::Parser( std::string const & conf ): _conf_file(conf)
{
	std::cout << _conf_file << ": Called constructor!" << std::endl;
}

Parser::Location	Parser::processLocation( std::string const & block )
{
	Location	ret;
	std::cout << GREEN "I'm in processLocation!" << std::endl;
	std::cout << block << RESET << std::endl;
	(void)block;
	return ret;
}
Parser::Server		Parser::processServer( std::string const & block, Parser::Server tempServer )
{
	std::cout << CYAN "I'm in processServer!" << std::endl;
	std::cout << block << RESET << std::endl;
	Server ret;
	(void)block;
	(void)tempServer;
	return ret;
}

std::string	Parser::extractNumbers(std::string const & str)
{
	std::string ret;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		if (std::isdigit(*it))
			ret.push_back(*it);
	}
	return ret;
}

//It's a simple function, but could be useful?
std::string	Parser::extractWord(std::string const & str, std::string const & key)
{
	std::string ret;
	bool	foundSpace = false;
	bool	foundKey = false;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		if (!foundKey && *it != ' ')
		{
			if ((*it == key.c_str()[0]) && std::string(it, it + (key.length())) == key)
			{
				foundKey = true;
				it += key.length() - 1;
				continue ;
			}
		}
		if (foundKey && !foundSpace)
		{
			if (std::isspace(*it))
			{
				foundSpace = true;
				continue ;
			}
		}
		if (foundKey && foundSpace && *it != ';')
			ret.push_back(*it);
	}
	return ret;
}


// Looks better than obtainSeverInfo, but I still have to remove ';' and I cannot catch more than one token
void	Parser::serverSetting(std::istringstream & iss, Server & server)
{
	std::string	token;
	while (iss >> token && token != "}")
	{
		std::cout << "token: " << token << std::endl;
		if (token == "server_name")
			iss >> server.name;
		else if (token == "root")
			iss >> server.root;
		else if (token == "listen")
			iss >> server.port;
	}
	std::cout << "after parsing: name: " << server.name << std::endl;
	std::cout << "root: " << server.root << std::endl;
	std::cout << "listen: " << server.port << std::endl;
}

void	Parser::obtainServerInfo(Parser::Server tempServer, std::string const & line)
{
	(void)tempServer;
	(void)line;
}


void	Parser::parseByLine(std::string const & line, int & listeningPort)
{
	static std::string serverBlock = ""; // Static variable to hold the current server block being parsed
	static std::string locationBlock = ""; // Static variable to hold the current location block being parsed
	static bool inServerBlock = false; // Flag to indicate whether we are currently inside a server block
	static bool inLocationBlock = false; // Flag to indicate whether we are currently inside a location block
	static Server tempServer;
	// static int	index = 0;
	(void)listeningPort;
	// std::cout << BLUE "Oh?\ninServerBlock: " << inServerBlock << "\ninLocationBlock: " << inLocationBlock << RESET <<  std::endl;

//When it's not in the sever block, but start location block, how treat it, error or exeption?

	// When you find the keyword "server", you will enter to the serverBlock
	if (line.find("server") != std::string::npos && line.find('{') != std::string::npos)
	{
		inServerBlock = true; // Start of a new block
		serverBlock += line; // Initialize the server block with the current line
		return;
	}
	else if (line.find("location") != std::string::npos && line.find('{') != std::string::npos)
	{
		inLocationBlock = true; // Start of a new block
		locationBlock += line; // Initialize the location block with the current line
		serverBlock += line; // Maybe I do not need it.
		return;
	}
	if (inServerBlock && !inLocationBlock)
	{
		serverBlock += line;
		obtainServerInfo(tempServer, line);
		// Check if we have reached the end of the block
		if (line.find('}') != std::string::npos)
		{
			std::replace(serverBlock.begin(), serverBlock.end(), '\r', '\n');
			// Now 'serverBlock' contains the entire block, process it here
			// Pass tempServer to obtain infos that has, then make the new one to put to _servers
			Server newServer = processServer(serverBlock, tempServer);
//			It's not implemented yet
//			this->_servers.insert(std::pair<int, Server>(tempServer.getIndex), newServer);
			// --- Initialization for the next block --- //
			inServerBlock = false; // End of the block
			serverBlock = ""; // Initialized string after obtaining server
		}
	}
	if (inLocationBlock)
	{
		serverBlock += line; // Concatenate the current line to the block
		locationBlock += line;

		// Check if we have reached the end of the block
		if (line.find('}') != std::string::npos)
		{
			std::replace(locationBlock.begin(), locationBlock.end(), '\r', '\n');
			// Now 'locationBlock' contains the entire block, process it here
			Location tempLocation= processLocation(locationBlock);
			// this->tempServer.locations.push_back(tempLocation);
			// --- Initialization for the next block --- //
			inLocationBlock = false; // End of the block
			locationBlock = ""; // Initialized string after obtaining location
		}
	}

	// --- If it's not in the location block, I can select from 2, extractNumber or extractWord... --- //
/*	else if (line.find("listen") != std::string::npos && inServerBlock)
	{
		//How will I control ports?
		std::cout << YELLOW "listen: " << line << std::endl;
		tempServer.port = extractNumbers(line);
		std::cout << tempServer.port << RESET << std::endl;
		serverBlock += line; // Concatenate the current line to the block
		return;
	}
	else if (line.find("server_name") != std::string::npos && inServerBlock)
	{
		//How will I control ports?
		std::cout << YELLOW "server_name: " << line << std::endl;
		tempServer.name = extractWord(line, "server_name");
		std::cout << tempServer.name << RESET << std::endl;
		serverBlock += line; // Concatenate the current line to the block
		return;
	}
	else if (line.find("server_name") != std::string::npos && inServerBlock)
	{
		//How will I control ports?
		std::cout << YELLOW "server_name: " << line << std::endl;
		tempServer.name = extractWord(line, "server_name");
		std::cout << tempServer.name << RESET << std::endl;
		serverBlock += line; // Concatenate the current line to the block
		return;
	}*/
	// When you find the keyword "location", you will enter to the locationBlock
// To check location blocks
}

bool	Parser::parse( void )
{
	std::ifstream	configFile(this->_conf_file.c_str());
	if (!configFile.is_open())
	{
		std::cerr << RED "Failed to open config file: " << _conf_file << RESET << std::endl;
		return false;
	}
	std::string	line;
	int	listningPort = -1;
	while (std::getline(configFile, line))
	{
		//parse line
		// std::cout << "\n>>> Input line: " GREEN <<line << RESET "\n";
		parseByLine(line, listningPort);
	}
	return true;
}

// --- Member functions --- //


std::ostream	&operator<<(std::ostream & os, Parser const & obj)
{
	(void)obj;
	os << "Parsed: ";
	return os;
}

