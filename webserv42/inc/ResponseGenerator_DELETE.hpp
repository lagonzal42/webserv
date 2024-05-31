#ifndef RESPONSEGENERATOR_DELETE_HPP
# define RESPONSEGENERATOR_DELETE_HPP

// This is a ResponseGenerator class for DELETE request

# include <iostream>
# include <string>
# include <sstream>
# include "colors.h"


class ResponseGeneratorDELETE
{
	public:
			ResponseGeneratorDELETE( void );
			~ResponseGeneratorDELETE( void );
			ResponseGeneratorDELETE( ResponseGeneratorDELETE const & src );
			ResponseGeneratorDELETE & operator=( ResponseGeneratorDELETE const & src );
	// --- Getter functions --- //

			static const char	* generateDeleteResponse(const std::string &fullPath);
	// --- Getter functions --- //


};
#endif //RESPONSEGENERATOR_DELETE_HPP
