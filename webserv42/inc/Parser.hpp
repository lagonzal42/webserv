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
				std::vector<std::string> methods; //method GET, POST OR DELETE
				std::string	cgi_path; //cgi_path
				std::string	upload_path; //upload_path
				std::string	redirect;
				bool		autoindex; // autoindex
				size_t		max_body_size; //max body size
			};
			struct Server
			{
				// int			index;
				std::string	name; //server_name, at this moment it's not necessarry but I will leave it
				std::string root; //root
				std::vector<Location> locations; //location /
				std::string	host; 
				std::string	port; //listen
				std::map<int, std::string> error_pages; //error_page int: error code, std::string path
				// Default constructor to initialize
				Server() : name(""), root(""), host(""), port("") {}
			};
//--- structs ---//
			Parser( void );
			~Parser( void );
			Parser( Parser const & src );
			Parser & operator=( Parser const & src );
			bool	parse( std::string const & conf );
	// --- Getter functions --- //
			std::map<std::string, Server>	const & getServers( void ) const;
			Server	const getServer( std::string const & port ) const;
			std::map<std::string, Server>	const & getDefServer( void ) const;
			std::string	const & getConfFile( void ) const;
			Location const getCurLocation( std::string const & path, std::string const & port ) const;
	// --- Getter functions --- //


	private:
			std::string	_conf_file;

			//As we don't know how many servers we will have, use a map to store them.
			//make a temp server and add to the map when we know port and index
			//MAYBE I will put std::string, like port number + server_name to identify.
			//20240523: change ths string to port number only.
			std::map<std::string, Server> _servers;
			std::map<std::string, Server> _serversDefault;
			void	serverSetting( std::istringstream & iss, Server & server );
			void	parseByLine( std::string const & line );
			// WIP
			Location	processLocation( std::string const & block );
			Server		processServer( Server tempServer );
			// Server		processServer( std::string const & block );
			void	obtainServerInfo(Server * tempServer, std::string const & line);

			std::string	extractNumbers(std::string const & str);
			std::string	extractWord(std::string const & str, std::string const & key);
			std::vector<std::string> obtainMethod(std::string const & line);
			size_t	obtainSizeFromStr(std::string const & num);

			std::string	getParentDirectory( const std::string & path ) const;

	protected:



};
std::ostream	&operator<<(std::ostream & os, Parser const & obj);

#endif //PARSER_HPP
