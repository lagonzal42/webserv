#include <fstream>
#include "Parser.hpp"
#include "Utils.hpp"
#include "colors.h"
#include <algorithm>
#include <iomanip>
#include <stdexcept>

/**
 * @author nozomi
 * @brief change the flow, construct by default_test.conf, and change if there are difference between default setting and ***.conf
*/



// --- Functions for Orthodox canonical class form --- //

Parser::Parser( void ): _conf_file("configurations/default.conf")
{
	// std::cout << _conf_file << ": Called default constructor!" << std::endl;
	Parser::parse(_conf_file);//error check
	// std::cout << _conf_file << ": Called initialization parser!" << std::endl;

}
Parser::~Parser( void )
{
	// std::cout << _conf_file << ": Called destructor, bye!" << std::endl;
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
	size_t	ret;
	std::istringstream iss(num);
	if (!(iss >> ret))
	{
		throw std::runtime_error("Failed to convert string to size_t in obtainSizeFromStr()" + num);
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
	// if (!this->_serversDefault.empty())
	// {
	// 	Parser::Server def = this->getDefServer().begin()->second;
	// 	std::vector<Parser::Location> & defLocations = def.locations;
	// 	for (std::vector<Parser::Location>::iterator it = defLocations.begin(); it != defLocations.end(); ++it)
	// 	{
	// 		if (block.find(it->name) != std::string::npos)
	// 		{
	// 			ret = *it;
	// 			break ;
	// 		} // block.find(it->name) != std::string::npos
	// 	} // std::vector<Parser::Location>::iterator it = defLocations.begin(); it != defLocations.end(); ++it
	// } // !this->_serversDefault.empty()

	std::istringstream iss(block);
	std::string line;
	ret.autoindex = false;
	ret.max_body_size = 0;

	while (std::getline(iss, line))
	{
		std::string num;

		if (!line.length() || line.find('}') != std::string::npos || line.find_first_not_of("}\t\v\n\r\f") == std::string::npos)
			continue ;
		std::string	info[9] = {"location", "method", "root", "autoindex", "upload_path", "redirect", "cgi_path", "index", "max_body_size"};
		int	i = 0;
		//CONFIRMATION NEEDED
		for (i = 0; i < 8; i++)
		{
			if (line.find(info[i]) != std::string::npos)
				break ;
		}
		switch (i)
		{
			case 0://location
					if (!ret.name.empty())
						throw std::runtime_error("processLocation: " + line);
					ret.name = extractWord(line, info[i]);//location / {   ==> I have to remove {
					break ;
			case 1://method
					if (!(ret.methods.empty()))
						throw std::runtime_error("processLocation: " + line);
					try
					{
						ret.methods = obtainMethod(line);
					}
					catch (const std::exception & e)
					{
						throw std::runtime_error("Unknown block type encountered: " + line);
					}
					break ;
			case 2://root
					if (!(ret.root.empty()))
						throw std::runtime_error("processLocation: " + line);
					ret.root = extractWord(line, info[i]);
					break ;
			case 3://autoindex ***I have to return bool
					if (line.find("on") != std::string::npos)
						ret.autoindex = true;
					break ;
			case 4://upload_path
					if (!(ret.upload_path.empty()))
						throw std::runtime_error("processLocation: " + line);
					ret.upload_path = extractWord(line, info[i]);
					break ;
			case 5://redirect
					if (!(ret.redirect.empty()))
						throw std::runtime_error("processLocation: " + line);
					ret.redirect = extractWord(line, info[i]);
					break ;
			case 6://cgi_path
					if (!(ret.cgi_path.empty()))
						throw std::runtime_error("processLocation: " + line);
					ret.cgi_path = extractWord(line, info[i]);
					break ;
			case 7://index
					if (!(ret.index.empty()))
						throw std::runtime_error("processLocation: " + line);
					ret.index = extractWord(line, info[i]);
					break ;
			case 8://max_body_size
					if (ret.max_body_size != 0)
						throw std::runtime_error("processLocation: " + line);
					num = Utils::extractNumbers(line);
					ret.max_body_size = obtainSizeFromStr(num);
					break ;
			default: throw std::runtime_error("Unknown block type encountered: " + line);
			// std::cout << RED << line << ": It's not in location block. I will put an exception" RESET << std::endl;
				// break ;
		}
	} // std::getline(iss, line)


	// check if the location is defined in the default server and it's not defined
	if (!this->_serversDefault.empty())
	{
		Parser::Server def = this->getDefServer().begin()->second;
		std::vector<Parser::Location> & defLocations = def.locations;
		for (std::vector<Parser::Location>::iterator it = defLocations.begin(); it != defLocations.end(); ++it)
		{
			if (ret.name == it->name)
			{
				// std::string	name;
				// std::string root; //root
				// std::string	index; //index
				// std::vector<std::string> methods; //method GET, POST OR DELETE
				// std::string	cgi_path; //cgi_path
				// std::string	upload_path; //upload_path
				// std::string	redirect;
				// bool		autoindex; // autoindex
				// size_t		max_body_size; //max b
				if (ret.root.empty() && !it->root.empty())
					ret.root = it->root;
				if (ret.index.empty() && !it->index.empty())
					ret.index = it->index;
				if (ret.methods.empty() && !it->methods.empty())
					ret.methods = it->methods;
				if (ret.cgi_path.empty() && !it->cgi_path.empty())
					ret.cgi_path = it->cgi_path;
				if (ret.upload_path.empty() && !it->upload_path.empty())
					ret.upload_path = it->upload_path;
				if (ret.redirect.empty() && !it->redirect.empty())
					ret.redirect = it->redirect;
				if (!ret.autoindex && it->autoindex)
					ret.upload_path = true;
				if (ret.max_body_size == 0 && !it->max_body_size != 0)
					ret.max_body_size = it->max_body_size;
				break ;
			} // block.find(it->name) != std::string::npos
		} // std::vector<Parser::Location>::iterator it = defLocations.begin(); it != defLocations.end(); ++it
	} // !this->_serversDefault.empty()



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
	if (!this->_serversDefault.empty())
	{
		Server def = this->getDefServer().begin()->second;
		ret.host = def.host;
		ret.locations = def.locations;
		ret.name = def.name;
		ret.port = def.port;
		// ret.root = def.root;
		ret.error_pages = def.error_pages;
	}
// --- DELETE --- //
	// std::cout << CYAN "TEST PRINT SERVER: \n";
	// std::cout << "host: " << ret.host << std::endl;
	// std::cout << "name: " << ret.name << std::endl;
	// std::cout << "port: " << ret.port << std::endl;
	// std::cout << "root: " << ret.root << "\n" RESET << std::endl;
// --- DELETE --- //

	if (!tempServer.host.empty())
		ret.host = tempServer.host;
	if (!tempServer.name.empty())
		ret.name = tempServer.name;
	if (!tempServer.port.empty())
		ret.port = tempServer.port;
	if (!tempServer.root.empty())
		ret.root = tempServer.root;



	if (!tempServer.error_pages.empty() && this->_serversDefault.empty())
		ret.error_pages = tempServer.error_pages;
	else if (!tempServer.error_pages.empty()/* && !(this->_serversDefault.empty())*/)
	{
		// ret.error_pages = tempServer.error_pages;
		std::map<int, std::string>::const_iterator tmp_it;
		for (tmp_it = tempServer.error_pages.begin(); tmp_it != tempServer.error_pages.end(); ++tmp_it)
		{
			// std::cout << "tmp: " << tmp_it->first << std::endl;
			std::map<int, std::string>::const_iterator ret_it;
			for (ret_it = ret.error_pages.begin(); ret_it != ret.error_pages.end(); ++ret_it)
			{
				// std::cout << "ret: " << ret_it->first << std::endl;
				if (ret_it->first == tmp_it->first)
					break ;
			}
			if (ret_it != ret.error_pages.end())
			{
				ret.error_pages[ret_it->first] = tmp_it->second;
			}
		}
	}
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
			// std::cout << BLUE "I added things that wasn't there! "  RESET << std::endl;
		}
	} // !this->_serversDefault.empty()
// --- DELETE --- //
	// std::cout << YELLOW "TEST PRINT SERVER: \n";
	// std::cout << "host: " << ret.host << std::endl;
	// std::cout << "name: " << ret.name << std::endl;
	// std::cout << "port: " << ret.port << std::endl;
	// std::cout << "root: " << ret.root << "\n" RESET << std::endl;
// --- DELETE --- //
	return ret;
}

//It's a simple function, but could be useful?
std::string	Parser::extractWord(std::string const & str, std::string const & key)
{
	std::string ret;
	bool	foundSpace = false;
	bool	foundKey = false;

	if (key == "")
	{
		for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		{
			if (*it != ';' && *it != '{' && *it != ' ')
				ret.push_back(*it);		
		}
		return ret;
	}
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
	} // std::string::const_iterator it = str.begin(); it != str.end(); ++it
	return ret;
}

bool	Parser::obtainServerInfo(Parser::Server * tempServer, std::string const & line)
{
	std::string	info[4] = {"listen", "server_name", "root", "error_page"};
	int	i = 0;
	std::string temp = line;
	std::replace(temp.begin(), temp.end(), '\r', ' ');

// This is not a best practice, make it better---
			std::istringstream line_ss(temp);
			std::string error_page;
			std::string path;
			int	error_code;
//
	for (i = 0; i < 4; i++)
	{
		if (temp.find(info[i]) != std::string::npos)
			break ;
	}
	if (i == 2 && !(tempServer->root.empty()))
	{
		return false;
		// throw std::runtime_error("You should not have more than one root: " + temp);
	}
	switch (i)
	{
	case 0:
			tempServer->port = Utils::extractNumbers(temp);
			tempServer->name = tempServer->port;
			break ;
	case 1:
			tempServer->host = extractWord(temp, "server_name");
			break ;
	case 2:
			tempServer->root = extractWord(temp, "root");
			break ;
	case 3:
			line_ss >> error_page >> error_code >> path;
			tempServer->error_pages[error_code] = extractWord(path, "");
			break ;

	default: throw std::runtime_error("Unknown block type encountered: " + temp);
		// std::cout << RED << temp << ": It's not in server block. I will put an exception" RESET << std::endl;
		break ;
	}
	return true;
}

	/*******/
	/*     */
	/* WIP */
	/*     */
	/*******/

// If you put something not GET, POST, nor DELETE, parse will get error and will not run the server
std::vector<std::string> Parser::obtainMethod(std::string const & line)
{
	std::vector<std::string> ret;
	size_t i = 0;
	size_t j = 0;
	while (i < line.length())
	{
		while (i < line.length() && std::isspace(line[i]))
			i++;
		j = i;
		while (j < line.length() && line[j] != ' ' && line[j] != ';')
			j++;
		if (i < line.length() && j < line.length() && j > i)
		{
			std::string method = line.substr(i, j - i);
			if (method != "method")
			{
				// std::cout << "method: !" << method  << "!" << std::endl;
				ret.push_back(method);
			}
		} // i < line.length() && j < line.length()
		i = j + 1;
	} // i < line.length()
	std::string	allowed_methods[3] = {"GET", "POST", "DELETE"};
	std::vector<std::string>::const_iterator it;
	for(it = ret.begin(); it!= ret.end(); ++it)
	{
		bool valid = false;
		for(i = 0; i < 3; ++i)
		{
			if(*it == allowed_methods[i])
			{
				valid = true;
				break ;
			}
		}
		if (!valid)
			throw std::runtime_error("Unknown method encountered: " + *it);
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
	if (line.find("server ") != std::string::npos && line.find('{') != std::string::npos)
	{
		inServerBlock = true; // Start of a new block
		serverBlock += line; // Initialize the server block with the current line
		serverBlock += "\n";
		return;
	}
	else if (line.find("server ") != std::string::npos && (line.find('{') == std::string::npos))
		throw std::runtime_error("The configuration file form is not correct" + line);
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
			// Now 'serverBlock' contains the entire block, process it here
			// Pass tempServer to obtain infos that has, then make the new one to put to _servers
			Server newServer = processServer(tempServer);
			// If it's initialized the default server.
			if (this->_serversDefault.empty())
				this->_serversDefault[tempServer.name] = newServer;
			else
			{
				if (this->_servers.find(tempServer.name) != this->_servers.end())
				{
					throw std::runtime_error("Duplicated port encountered: " + tempServer.name);
				}
				this->_servers[tempServer.name] = newServer;
			}
			// --- Initialization for the next block --- //
			inServerBlock = false;
			serverBlock = "";
			tempServer = Server();
			return ;
		}
		if (!obtainServerInfo(&tempServer, line))
			throw std::runtime_error("obtainServerInfo false: " + line);
	} // inServerBlock && !inLocationBlock
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
	} // inLocationBlock
}

bool	Parser::parse( std::string const & conf )
{
	std::ifstream	configFile(conf.c_str());
	if (!configFile.is_open())
	{
		throw std::runtime_error("Failed to open config file: " + conf);
		// std::cerr << RED "Failed to open config file: " << _conf_file << RESET << std::endl;
	}

	std::string	line;
	while (std::getline(configFile, line))
	{
		// parse line
		parseByLine(line);
	}
	configFile.close();
	if (!(this->_servers.empty()) && (_serversDefault.begin()->second.port == "") && (_servers.begin()->second.port == ""))
	{
		throw std::runtime_error("No port in file: " + conf);
		// return false;
	}
	std::map<std::string, Server>::const_iterator it;
	for (it = _servers.begin(); it != _servers.end(); it++)
	{
		size_t i = 0;
		std::vector<Location> copy = it->second.locations;
		for (; i != copy.size(); i++ )
		{
			size_t h = i + 1;
			for (; h != copy.size(); h++)
			{
				if (copy[i].root == copy[h].root)
					throw (std::runtime_error("Different locations can't have same root"));
			}
		}
	}
	return true;
}

std::string	Parser::getParentDirectory( const std::string & path ) const
{
	// Find the last occurrence of '/'
	std::string::size_type pos = path.find_last_of('/');

	// If '/' is found and it's not the first character, return the substring up to the last '/'
	if (pos != std::string::npos && pos > 0) {
		return path.substr(0, pos + 1);
	}

	// If the path is just a root directory or has no slashes, return "/"
	return "/";
}



	// --- Getter functions --- //
std::map<std::string, Parser::Server>	const & Parser::getServers( void ) const
{
	return this->_servers;
}

Parser::Server	const Parser::getServer( std::string const & port ) const
{
	Parser::Server ret;
	// std::cout << YELLOW << "Im in the getServer! " RESET;
	// std::cout << "port: " << port << std::endl;
	std::map<std::string, Parser::Server> servers = this->getServers();

	std::map<std::string, Parser::Server>::const_iterator server_iter = servers.find(port);
	if (server_iter == servers.end())
	{
		// No server found for the given port
		std::cout << RED << "Any server detected from getServer!" RESET << std::endl;
		return ret;
	}
	std::cout << YELLOW "Detected Server: port: \"" << port << "\" in getServer" RESET << std::endl;
	return server_iter->second;
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
	// std::cout << BLUE << "Im in the getCurLocation! " RESET;
	// std::cout << "path: " << path << " port: " << port << std::endl;
	std::string parentPath = getParentDirectory(path);
	// std::cout << LGREEN "parentPath: " RESET << parentPath << std::endl;
	const Parser::Server &curServer = this->getServer(port);
	std::vector<Parser::Location>::const_iterator location_iter;

	// --- 1st step: Find exact match for the location using the path
	for (location_iter = curServer.locations.begin(); location_iter != curServer.locations.end(); ++location_iter)
	{
		const Parser::Location &location = *location_iter;
		if (location.root != "" && location.root == parentPath)
		{
			std::cout << YELLOW "Detected Location: path: \"" << parentPath << "\" in getCurLocation" RESET << std::endl;
			return location;
		}
// *** There is doubt!!! ***//
		else if (location.root == "" && /*location.name == parentPath*/parentPath == "/")
		{
			std::cout << YELLOW "Detected Location: path: \"" << parentPath << "\" in getCurLocation" RESET << std::endl;
			return location;
		}
	}
	
	// --- 2st step: No exact match found, look for the closest matching path
	const Parser::Location* bestMatch = NULL;
	size_t longestMatchLength = 0;
	for (location_iter = curServer.locations.begin(); location_iter != curServer.locations.end(); ++location_iter)
	{
		const Parser::Location &location = *location_iter;
		// Check if location.root is a prefix of parentPath
		if (parentPath.find(location.root) == 0)
		{
			size_t matchLength = location.root.length();
			if (matchLength > longestMatchLength/* && path[location.root.length()] == '/'*/)
			{
				longestMatchLength = matchLength;
				bestMatch = &location;
				// std::cout << "bestMatch: " << bestMatch->name << std::endl;
			}
		}
	}
	if (bestMatch)
	{
		// std::cout << YELLOW "I found the closest match: " << bestMatch->root << "!!!!" RESET << std::endl;
		return *bestMatch;
	}
	else
	{
		return *(curServer.locations.begin());
	}

	// --- If this line is reached, this folder(parentPath) doesn't exist.
	throw std::runtime_error("the root has not been encontered");
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
		os << std::setw(15) << "error_pages: " << "\n";
		std::map<int, std::string>::const_iterator error_it;
		for (error_it = server.error_pages.begin(); error_it != server.error_pages.end(); ++error_it)
		{
			os << std::setw(18) << "<" << error_it->first << ">" << error_it->second << "\n";
		}
		
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
