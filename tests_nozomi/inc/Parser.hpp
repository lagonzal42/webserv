#ifndef PARSER_HPP
# define PARSER_HPP

//how about if I put port number and index in map
//std::map<int, Server>

# include <map>
# include <vector>
# include <iostream>
# include <string>
# include <sstream>

// Read the configuration file line by line.
// Parse each line to extract relevant information such as server settings, locations, methods, etc.
// Store the parsed information in appropriate data structures.
// Use the parsed information as needed in your program.

class Parser
{
	public:
			Parser( void );
			Parser( std::string const & conf );
			~Parser( void );
			Parser( Parser const & src );
			Parser & operator=( Parser const & src );
			bool	parse( void );

	private:
//--- structs ---//
			struct Location
			{
				std::string	name;
				std::string root;
				std::string methods;
//				std::vector<std::string> methods; //GET, POST OR DELETE
				std::string	cgi_path;
				std::string	upload_path;
				std::string	redirect;
			};
			struct Server
			{
				int			index;
				std::string	name;
				std::string root;
				std::vector<Location> locations;
				std::string	host;
				int			port;
			};
//--- structs ---//
			std::string	_conf_file;
			//As we don't know how many servers we will have, use a map to store them.
			//make a temp server and add to the map when we know port and index
			std::map<int, Server> _servers;
			void	serverSetting( std::istringstream & iss, Server & server );
			void	parseByLine( std::string const & line, int & listeningPort );
			// WIP
			Location	processLocation( std::string const & block );
			Server		processServer( std::string const & block );


	protected:



};
std::ostream	&operator<<(std::ostream & os, Parser const & obj);

#endif //PARSER_HPP
