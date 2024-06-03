#ifndef RESPONSEGENERATOR_DELETE_HPP
# define RESPONSEGENERATOR_DELETE_HPP

// This is a ResponseGenerator class for DELETE request

# include <iostream>
# include <string>
# include <sstream>
# include "Request.hpp"
# include "Parser.hpp"
# include "colors.h"


class ResponseGeneratorDELETE
{
	public:
			ResponseGeneratorDELETE( void );
			~ResponseGeneratorDELETE( void );
			ResponseGeneratorDELETE( ResponseGeneratorDELETE const & src );
			ResponseGeneratorDELETE & operator=( ResponseGeneratorDELETE const & src );
	// --- Getter functions --- //
			// I don't know why does work well with std::string, and not with char *
			static std::string generateDeleteResponse(Request & req, const Parser::Location & currentLoc, const std::string &fullPath);
			static std::string generateHttpResponse(const std::string &status, const std::string &title, const std::string &message);

			// static const char	* generateDeleteResponse(Request & req, const Parser::Location & currentLoc, const std::string &fullPath);
	// --- Getter functions --- //


};
#endif //RESPONSEGENERATOR_DELETE_HPP
