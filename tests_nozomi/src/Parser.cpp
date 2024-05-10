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
//=== TEST ===//
	std::cout << GREEN "I'm in processLocation!" << std::endl;
	std::cout << block << RESET << std::endl;
//=== TEST ===//

	std::istringstream iss(block);
	std::string line;
	ret.autoindex = false;

	// std::string	name;
	// std::string root; //root
	// std::string	index; //index
	// // std::string methods;
	// std::vector<std::string> methods; //method GET, POST OR DELETE
	// std::string	cgi_path; //cgi_path
	// std::string	upload_path; //upload_path
	// std::string	redirect;
	// bool		autoindex; // autoindex

	while (std::getline(iss, line))
	{
		if (!line.length() || line.find('#') != std::string::npos || line.find('}') != std::string::npos || line.find_first_not_of("}\t\v\n\r\f") == std::string::npos)
			continue ;
		std::string	info[7] = {"location", "method", "root", "autoindex", "upload_path", "cgi_path", "index"};
		int	i = 0;
		for (i = 0; i < 7; i++)
		{
			if (line.find(info[i]) != std::string::npos)
				break ;
		}
		switch (i)
		{
		case 0://location
				ret.name = extractWord(line, info[i]);//location / {   ==> I have to remove {
				break ;
		case 1://method	
				ret.methods = obtainMethod(line);
				break ;
		case 2://root
				ret.root = extractWord(line, info[i]);
				break ;
		case 3://autoindex ***I have to return bool
				if (line.find("on") != std::string::npos)
					ret.autoindex = true;
				break ;
		case 4://upload_path
				ret.upload_path = extractWord(line, info[i]);
				break ;
		case 5://cgi_path
				ret.cgi_path = extractWord(line, info[i]);
				break ;
		case 6://index
				ret.index = extractWord(line, info[i]);
				break ;
		default: std::cout << RED << line << ": It's not in location block. I will put an exception" RESET << std::endl;
			break ;
		}
	}
//=== TEST ===//
	std::cout << "ret.name: " << ret.name << std::endl;
	std::cout << "ret.root: " << ret.root << std::endl;
	std::cout << "ret.upload_path: " << ret.upload_path << std::endl;
	std::cout << "ret.cgi_path: " << ret.cgi_path << std::endl;
	std::cout << "ret.index: " << ret.index << std::endl;
	std::cout << "ret.methods: ";
	for (std::vector<std::string>::const_iterator it = ret.methods.begin(); it != ret.methods.end(); ++it)
		std::cout << *it << " ";
	std::cout << "\nret.autoindex: " << ret.autoindex << std::endl;
//=== TEST ===//

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

// std::string	name; //server_name
// std::string root; //root
// std::vector<Location> locations; //location /
// std::string	host; 
// std::string	port; //listen
void	Parser::obtainServerInfo(Parser::Server * tempServer, std::string const & line)
{

	std::string	info[3] = {"listen", "server_name", "root"};
	int	i = 0;
	for (i = 0; i < 3; i++)
	{
		if (line.find(info[i]) != std::string::npos)
			break ;
	}
	switch (i)
	{
	case 0:
			tempServer->port = extractNumbers(line);
			break ;
	case 1:
			tempServer->name = extractWord(line, "server_name");
			break ;
	case 2:
			tempServer->root = extractWord(line, "root");
			break ;
	default: std::cout << RED << line << ": It's not in server block. I will put an exception" RESET << std::endl;
		break ;
	}
}

std::vector<std::string> Parser::obtainMethod(std::string const & line)
{
	std::vector<std::string>	ret;
	std::string	methods[3] = {"GET", "POST", "DELETE"};
	int	i;
	for (i = 0; i < 3; i++)
	{
		if (line.find(methods[i]) != std::string::npos)
			ret.push_back(methods[i]);
	}
	return ret;
}


void	Parser::parseByLine(std::string const & line, int & listeningPort)
{
	static std::string serverBlock = ""; // Static variable to hold the current server block being parsed
	static std::string locationBlock = ""; // Static variable to hold the current location block being parsed
	static bool inServerBlock = false; // Flag to indicate whether we are currently inside a server block
	static bool inLocationBlock = false; // Flag to indicate whether we are currently inside a location block
	static Server tempServer;
	(void)listeningPort;

	// Skip if it's empty line
	if (!line.length() || line.find_first_not_of("\t\v\n\r\f") == std::string::npos)
		return ;
	// When you find the keyword "server", you will enter to the serverBlock
	if (line.find("server") != std::string::npos && line.find('{') != std::string::npos)
	{
		inServerBlock = true; // Start of a new block
		serverBlock += line; // Initialize the server block with the current line
		serverBlock += "\n"; // 
		return;
	}
	else if (line.find("location") != std::string::npos && line.find('{') != std::string::npos)
	{
		inLocationBlock = true; // Start of a new block
		locationBlock += line; // Initialize the location block with the current line
		serverBlock += line; // Maybe I do not need it.		locationBlock += line; // Initialize the location block with the current line
		locationBlock += "\n"; // 
		serverBlock += "\n"; // 
		return;
	}
	if (inServerBlock && !inLocationBlock)
	{
		serverBlock += line; 
		serverBlock += "\n"; // 
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
			tempServer = Server(); // Initialized server structure
			return ;
		}
		obtainServerInfo(&tempServer, line);
	}
	if (inLocationBlock)
	{
		serverBlock += line; // Concatenate the current line to the block
		locationBlock += line;
		locationBlock += "\n"; // 
		serverBlock += "\n"; // 
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

	// --- Getter functions --- //
std::map<int, Parser::Server>	const & Parser::getServers( void ) const
{
	return this->_servers;
}
std::string	const & Parser::getConfFile( void ) const
{
	return this->_conf_file;
}
	// --- Getter functions --- //

// --- Member functions --- //


std::ostream	&operator<<(std::ostream & os, Parser const & obj)
{
	os << "_conf_file: " << obj.getConfFile() << "\n_servers:\n";
	return os;
}

