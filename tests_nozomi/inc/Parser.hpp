#ifndef PARSER_HPP
# define PARSER_HPP

//how about if I put port number and index in map
//std::map<int, Server>

# include <map>
# include <vector>
# include <iostream>
# include <string>
# include <sstream>
# include <bitset>

// Read the configuration file line by line.
// Parse each line to extract relevant information such as server settings, locations, methods, etc.
// Store the parsed information in appropriate data structures.
// Use the parsed information as needed in your program.

// # define METHOD_GET (1<<0) // 0000 0000 0001
// # define METHOD_POST (1<<2) // 0000 0000 0100
// # define METHOD_DELETE (1<<4) // 0000 0001 0000

class Parser
{
	public:
//--- structs ---//
			struct Location
			{
				std::string	name;
				std::string root; //root
				std::string	index; //index
				// std::string methods;
				std::vector<std::string> methods; //method GET, POST OR DELETE
				// int			methods;
				std::string	cgi_path; //cgi_path
				std::string	upload_path; //upload_path
				std::string	redirect;
				bool		autoindex; // autoindex
			};
			struct Server
			{
				// int			index;
				std::string	name; //server_name
				std::string root; //root
				std::vector<Location> locations; //location /
				std::string	host; 
				std::string	port; //listen
				// Default constructor
				Server() : name(""), root(""), host(""), port("") {}
			};
//--- structs ---//
			Parser( void );
			Parser( std::string const & conf );
			~Parser( void );
			Parser( Parser const & src );
			Parser & operator=( Parser const & src );
			bool	parse( void );
			std::map<int, Server>	const & getServers( void ) const;
			std::string	const & getConfFile( void ) const;

	private:
			std::string	_conf_file;

			//As we don't know how many servers we will have, use a map to store them.
			//make a temp server and add to the map when we know port and index
			//MAYBE I will put std::string, like port number + server_name to identify.
			std::map<int, Server> _servers;
			void	serverSetting( std::istringstream & iss, Server & server );
			void	parseByLine( std::string const & line, int & listeningPort );
			// WIP
			Location	processLocation( std::string const & block );
			Server		processServer( std::string const & block, Server tempServer );
			// Server		processServer( std::string const & block );
			void	obtainServerInfo(Server * tempServer, std::string const & line);

			std::string	extractNumbers(std::string const & str);
			std::string	extractWord(std::string const & str, std::string const & key);
			std::vector<std::string> obtainMethod(std::string const & line);

	protected:



};
std::ostream	&operator<<(std::ostream & os, Parser const & obj);

#endif //PARSER_HPP
