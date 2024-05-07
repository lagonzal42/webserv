#include <fstream>
#include "Parser.hpp"
#include "colors.h"

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

Location	Parser::processLocation( std::string const & block )
{
	Location	ret;
	return ret;
}
Server		Parser::processServer( std::string const & block )
{
	Server ret;
	return ret;
}


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

void	Parser::parseByLine(std::string const & line, int & listeningPort)
{
	static std::string serverBlock; // Static variable to hold the current server block being parsed
	static std::string locationBlock; // Static variable to hold the current location block being parsed
	static bool inServerBlock = false; // Flag to indicate whether we are currently inside a server block
	static bool inLocationBlock = false; // Flag to indicate whether we are currently inside a location block
	// static int	index = 0;
	(void)listeningPort;

	// When you find the keyword "server", you will enter to the serverBlock
	if (line.find("server") != std::string::npos && line.find('{') != std::string::npos)
	{
		inServerBlock = true; // Start of a new block
		serverBlock = line; // Initialize the server block with the current line
		return;
	}
	else if (line.find("listen") != std::string::npos)
	{
		//How will I control ports?
		;
	}
	// When you find the keyword "location", you will enter to the locationBlock
	else if (line.find("location") != std::string::npos && line.find('{') != std::string::npos)
	{
		inLocationBlock = true; // Start of a new block
		locationBlock = line; // Initialize the location block with the current line
		serverBlock += line; // Maybe I do not need it.
		return;
	}
	if (inLocationBlock)
	{
		locationBlock += line; // Concatenate the current line to the block

		// Check if we have reached the end of the block
		if (line.find('}') != std::string::npos)
		{
			Location curLocation;
			inLocationBlock = false; // End of the block
			// Now 'locationBlock' contains the entire block, process it here
			// processLocation(locationBlock, curLocation);
		}
	}
	if (!inLocationBlock && inServerBlock)
	{
		serverBlock += line; // Concatenate the current line to the block
		// Check if we have reached the end of the block
		if (line.find('}') != std::string::npos)
		{
			inServerBlock = false; // End of the block
			// Now 'serverBlock' contains the entire block, process it here
			// processServer(serverBlock);


			//It's not implemented yet
//			Server temp = processServer(serverBlock);
//			this->_servers.insert(std::pair<int, Server>(temp.getIndex), temp);
		}
	}
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
		std::cout << line << "\n";
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

