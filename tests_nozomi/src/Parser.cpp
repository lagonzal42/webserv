#include <fstream>
#include "Parser.hpp"
#include "colors.h"
#include <algorithm>
#include <iomanip>
#include <stdexcept>

/**
 * @author nozomi
 * @brief change the flow, construct by default_test.conf, and change if there are difference between default setting and ***.conf
*/



// --- Functions for Orthodox canonical class form --- //

Parser::Parser( void ): _conf_file("configurations/webserv.conf")
{
	std::cout << _conf_file << ": Called default constructor!" << std::endl;
	Parser::parse(_conf_file);//error check
	std::cout << _conf_file << ": Called initialization parser!" << std::endl;
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

// Parser::Parser( std::string const & conf ): _conf_file(conf)
// {
// 	std::cout << _conf_file << ": Called constructor!" << std::endl;
// }

size_t	Parser::obtainSizeFromStr(std::string const & num)
{
	size_t	ret = 0;
	std::istringstream iss(num);
	if (!(iss >> ret))
	{
		throw std::runtime_error("Failed to convert string to size_t in obtainSizeFromStr()");
		// std::cerr << "convert error." << std::endl;
		// return 1; //put exception, end will never get return when it returns error
	}
	return ret;
}

Parser::Location	Parser::processLocation( std::string const & block )
{
	Location	ret;
//=== TEST ===//
	// std::cout << GREEN "I'm in processLocation!" << std::endl;
	// std::cout << block << RESET << std::endl;
//=== TEST ===//

	//check if the location is defined in the default server
	if (!this->_serversDefault.empty())
	{
		Parser::Server def = this->getDefServer().begin()->second;
		std::vector<Parser::Location> & defLocations = def.locations;
		for (std::vector<Parser::Location>::iterator it = defLocations.begin(); it != defLocations.end(); ++it)
		{
			if (block.find(it->name) != std::string::npos)
			{
				std::cout << GREEN "I found it! " << it->name << std::endl;
				std::cout << RESET << block << std::endl;
				ret = *it;
				std::cout << "ret.name: " << ret.name << std::endl;
				std::cout << "ret.root: " << ret.root << std::endl;
				std::cout << "ret.upload_path: " << ret.upload_path << std::endl;
				std::cout << "ret.cgi_path: " << ret.cgi_path << std::endl;
				std::cout << "ret.index: " << ret.index << std::endl;
				std::cout << "ret.methods: ";
				for (std::vector<std::string>::const_iterator it = ret.methods.begin(); it != ret.methods.end(); ++it)
					std::cout << *it << " ";
				std::cout << "\nret.autoindex: " << ret.autoindex << std::endl;
				std::cout << "ret.max_body_size: " << ret.max_body_size << std::endl;
				break ;
			}
		}
	}

	std::istringstream iss(block);
	std::string line;
	ret.autoindex = false;
	ret.max_body_size = 0;

	// std::string	name;
	// std::string root; //root
	// std::string	index; //index
	// // std::string methods;
	// std::vector<std::string> methods; //method GET, POST OR DELETE
	// std::string	cgi_path; //cgi_path
	// std::string	upload_path; //upload_path
	// std::string	redirect;
	// bool		autoindex; // autoindex
	// size_t		max_body_size; //max body size
	while (std::getline(iss, line))
	{
		std::string num;

		if (!line.length() || line.find('}') != std::string::npos || line.find_first_not_of("}\t\v\n\r\f") == std::string::npos)
			continue ;
		std::string	info[8] = {"location", "method", "root", "autoindex", "upload_path", "cgi_path", "index", "max_body_size"};
		int	i = 0;
		for (i = 0; i < 8; i++)
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
			case 7://max_body_size
					num = extractNumbers(line);
					ret.max_body_size = obtainSizeFromStr(num);
					break ;
			default: throw std::runtime_error("Unknown block type encountered: " + line);
			// std::cout << RED << line << ": It's not in location block. I will put an exception" RESET << std::endl;
				// break ;
		}
	}
//=== TEST ===//
	// std::cout << "ret.name: " << ret.name << std::endl;
	// std::cout << "ret.root: " << ret.root << std::endl;
	// std::cout << "ret.upload_path: " << ret.upload_path << std::endl;
	// std::cout << "ret.cgi_path: " << ret.cgi_path << std::endl;
	// std::cout << "ret.index: " << ret.index << std::endl;
	// std::cout << "ret.methods: ";
	// for (std::vector<std::string>::const_iterator it = ret.methods.begin(); it != ret.methods.end(); ++it)
	// 	std::cout << *it << " ";
	// std::cout << "\nret.autoindex: " << ret.autoindex << std::endl;
	// std::cout << "ret.max_body_size: " << ret.max_body_size << std::endl;
//=== TEST ===//

	return ret;
}

Parser::Server		Parser::processServer( Parser::Server tempServer )
{
//=== TEST ===//
	// std::cout << CYAN "I'm in processServer!" << std::endl;
	// std::cout << block << RESET << std::endl;
//=== TEST ===//
	Server ret;
	if (tempServer.port.empty() && !this->_serversDefault.empty())
	{
		tempServer.port = this->getDefServer().begin()->second.port;
	}
	ret.host = tempServer.host;
	ret.locations = tempServer.locations;
	//to check if the locations incudes things that should not be empty
	if (!this->_serversDefault.empty())
	{
		Parser::Server def = this->getDefServer().begin()->second;
		std::vector<Parser::Location> & defLocations = def.locations;
		std::vector<Parser::Location>::iterator retIt;
		std::vector<Parser::Location>::iterator defIt;

		for (defIt = defLocations.begin(); defIt != defLocations.end(); ++defIt)
		{
			for (retIt = ret.locations.begin(); retIt != ret.locations.end(); ++retIt)
			{
				if (defIt->name == retIt->name)
					break ;
			}
			if (retIt != ret.locations.end())
				continue ;
			ret.locations.push_back(*defIt);
			std::cout << BLUE "I added things that wasn't there! "  RESET << std::endl;
		}
	}
	ret.name = tempServer.name;
	ret.port = tempServer.port;
	ret.root = tempServer.root;
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
		if (foundKey && foundSpace && *it != ';' && *it != '{' && *it != ' ')
			ret.push_back(*it);
	}
	return ret;
}

void	Parser::obtainServerInfo(Parser::Server * tempServer, std::string const & line)
{
	std::string	info[3] = {"listen", "server_name", "root"};
	int	i = 0;
	std::string temp = line;
	std::replace(temp.begin(), temp.end(), '\r', ' ');

	for (i = 0; i < 3; i++)
	{
		if (temp.find(info[i]) != std::string::npos)
			break ;
	}
	switch (i)
	{
	case 0:
			tempServer->port = extractNumbers(temp);
			tempServer->name = tempServer->port;
			break ;
	case 1:
	//as I changed the map string to only port number, also it's changed
			tempServer->host = extractWord(temp, "server_name");
			// if (tempServer->name != "")
			// 	tempServer->name = tempServer->host + ":" + tempServer->port;
			// else
			// 	tempServer->name = tempServer->host;
			break ;
	case 2:
			tempServer->root = extractWord(temp, "root");
			break ;
	default: throw std::runtime_error("Unknown block type encountered: " + temp);
		// std::cout << RED << temp << ": It's not in server block. I will put an exception" RESET << std::endl;
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


void	Parser::parseByLine(std::string const & line)
{
	static std::string serverBlock = ""; // Static variable to hold the current server block being parsed
	static std::string locationBlock = ""; // Static variable to hold the current location block being parsed
	static bool inServerBlock = false; // Flag to indicate whether we are currently inside a server block
	static bool inLocationBlock = false; // Flag to indicate whether we are currently inside a location block
	static Server tempServer;

	// Skip if it's empty line
	if (!line.length() || line.find_first_not_of("\t\v\n\r\f") == std::string::npos || line.find('#') != std::string::npos)
		return ;

	// When you find the keyword "server", you will enter to the serverBlock
	if (line.find("server") != std::string::npos && line.find('{') != std::string::npos)
	{
		inServerBlock = true; // Start of a new block
		serverBlock += line; // Initialize the server block with the current line
		serverBlock += "\n";
		return;
	}
	else if (line.find("location") != std::string::npos && line.find('{') != std::string::npos)
	{
		inLocationBlock = true; // Start of a new block
		locationBlock += line; // Initialize the location block with the current line
		serverBlock += line; // Maybe I do not need it.		locationBlock += line; // Initialize the location block with the current line
		locationBlock += "\n";
		serverBlock += "\n";
		return;
	}
	if (inServerBlock && !inLocationBlock)
	{
		serverBlock += line; 
		serverBlock += "\n"; // 
		// Check if we have reached the end of the block
		if (line.find('}') != std::string::npos)
		{
		//=== TEST ===// I can remove this when I don't print
			// std::replace(serverBlock.begin(), serverBlock.end(), '\r', '\n');
		//=== TEST ===//
		//=== TEST ===//
			// std::cout << YELLOW << std::endl;
			// std::cout << "tempServer.name: " << tempServer.name << std::endl;
			// std::cout << "tempServer.root: " << tempServer.root << std::endl;
			// std::cout << "tempServer.host: " << tempServer.host << std::endl;
			// std::cout << "tempServer.port: " << tempServer.port << RESET << std::endl;
		//=== TEST ===//
			// Now 'serverBlock' contains the entire block, process it here
			// Pass tempServer to obtain infos that has, then make the new one to put to _servers
			Server newServer = processServer(tempServer);
			// If it's initialized the default server.
			if (this->_serversDefault.empty())
				this->_serversDefault[tempServer.name] = newServer;
			else
				this->_servers[tempServer.name] = newServer;
			// --- Initialization for the next block --- //
			inServerBlock = false;
			serverBlock = "";
			tempServer = Server();
			return ;
		}
		obtainServerInfo(&tempServer, line);
	}
	if (inLocationBlock)
	{
		serverBlock += line; // Concatenate the current line to the block
		locationBlock += line;
		locationBlock += "\n";
		serverBlock += "\n";
		// Check if we have reached the end of the block
		if (line.find('}') != std::string::npos)
		{
			std::replace(locationBlock.begin(), locationBlock.end(), '\r', '\n');
			Location tempLocation = processLocation(locationBlock);
			tempServer.locations.push_back(tempLocation);
			// --- Initialization for the next block --- //
			inLocationBlock = false;
			locationBlock = "";
		}
	}
}

bool	Parser::parse( std::string const & conf )
{
	std::ifstream	configFile(conf.c_str());
	if (!configFile.is_open())
	{
		throw std::runtime_error("Failed to open config file: " + conf);
		// std::cerr << RED "Failed to open config file: " << _conf_file << RESET << std::endl;
		// return false;
	}
	std::string	line;
	while (std::getline(configFile, line))
	{
		//parse line
		// std::cout << "\n>>> Input line: " GREEN <<line << RESET "\n";
		parseByLine(line);
	}
	return true;
}

	// --- Getter functions --- //
std::map<std::string, Parser::Server>	const & Parser::getServers( void ) const
{
	return this->_servers;
}

std::map<std::string, Parser::Server>	const & Parser::getDefServer( void ) const
{
	return this->_serversDefault;
}

std::string	const & Parser::getConfFile( void ) const
{
	return this->_conf_file;
}

/**
 * @note return the current location
*/
Parser::Location const Parser::getCurLocation( std::string const & path, std::string const & port ) const
{
	//I think I will recieve path ---  "/upload/img"   port ---  "8080"
	Parser::Location ret;

	//try to find server by server port number
	std::map<std::string, Parser::Server> servers = this->getServers();
	std::map<std::string, Parser::Server>::const_iterator server_iter = servers.find(port);
	if (server_iter == servers.end())
	{
		// If no server found for the given port, return as a error
		return ret;
	}
	std::cout << YELLOW "I found " << port << "!!!!" RESET << std::endl;

	const Parser::Server &curServer = server_iter->second;
	std::vector<Parser::Location>::const_iterator location_iter;
	// Find the location in the server using the path
	for (location_iter = curServer.locations.begin(); location_iter != curServer.locations.end(); ++location_iter)
	{
		const Parser::Location &location = *location_iter;
		//if it has the exactly same path, it's fine. but if it's not, I need to search the closest one
		if (location.root == path)
		{
			std::cout << YELLOW "I found " << path << "!!!!" RESET << std::endl;
			return location;
		}
	}
	//OK, now I have to look for the closest matching path
	const Parser::Location* bestMatch = NULL;
	size_t longestMatchLength = 0;

	for (location_iter = curServer.locations.begin(); location_iter != curServer.locations.end(); ++location_iter)
	{
		const Parser::Location &location = *location_iter;
		// Check if location.root is a prefix of path
		if (path.find(location.root) == 0)
		{
			size_t matchLength = location.root.length();
			if (matchLength > longestMatchLength)
			{
				longestMatchLength = matchLength;
				bestMatch = &location;
			}
		}
	}
	if (bestMatch)
	{
		std::cout << YELLOW "I found the closest match: " << bestMatch->root << "!!!!" RESET << std::endl;
		return *bestMatch;
	}
	return ret;

}


	// --- Getter functions --- //

// --- Member functions --- //


std::ostream &operator<<(std::ostream &os, const Parser &obj)
{
	os << std::setw(12) << "_conf_file: " << obj.getConfFile() << YELLOW "\n" << std::setw(12) << "_servers: " << "\n";

	// Iterate through the servers
	std::map<std::string, Parser::Server>::const_iterator server_iter;
	for (server_iter = obj.getServers().begin(); server_iter != obj.getServers().end(); ++server_iter)
	{
		const Parser::Server &server = server_iter->second;

		os << "   " YELLOW << std::setfill('-') << std::setw(45) << "\n";
		os << std::setfill(' ') << std::setw(15) << "Server name: " << server.name << "\n";
		os << std::setw(15) << "host: " << server.host << "\n";
		os << std::setw(15) << "port: " << server.port << "\n";
		os << std::setw(15) << "root: " << server.root << "\n";
		os << std::setw(15) << "locations: " << "\n   ";
		os << std::setfill('-') << std::setw(45) << "\n";

		// Iterate through the locations of the current server
		std::vector<Parser::Location>::const_iterator location_iter;
		for (location_iter = server.locations.begin(); location_iter != server.locations.end(); ++location_iter)
		{
			const Parser::Location &location = *location_iter;
			os << std::setfill(' ') << GREEN << std::setw(18) << "name: " << location.name << "\n";
			os << std::setw(18) << "methods: ";
			for (std::vector<std::string>::const_iterator it = location.methods.begin(); it != location.methods.end(); ++it)
				os << *it << " ";
			os << "\n";
			os << std::setw(18) << "root: " << location.root << "\n";
			os << std::setw(18) << "index: " << location.index << "\n";
			os << std::setw(18) << "cgi_path: " << location.cgi_path << "\n";
			os << std::setw(18) << "upload_path: " << location.upload_path << "\n";
			os << std::setw(18) << "redirect: " << location.redirect << "\n";
			os << std::setw(18) << "autoindex: " << location.autoindex << "\n";
			os << std::setw(18) << "max_body_size: " << location.max_body_size << "\n   ";
			os << std::setfill('*') << std::setw(50) << RESET "\n";
		}
	}
	os << std::setfill(' ');

	return os;
}
