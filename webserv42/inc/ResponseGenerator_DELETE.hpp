#ifndef RESPONSEGENERATOR_DELETE_HPP
# define RESPONSEGENERATOR_DELETE_HPP

// This is a ResponseGenerator class for DELETE request

# include <iostream>
# include <string>
# include <sstream>


class ResponseGenerator
{
	public:
			ResponseGeneratorDELETE( void );
			~ResponseGeneratorDELETE( void );
			ResponseGeneratorDELETE( ResponseGeneratorDELETE const & src );
			ResponseGeneratorDELETE & operator=( ResponseGeneratorDELETE const & src );
	// --- Getter functions --- //

			std::string generateDeleteResponse(const std::string &fullPath);
	// --- Getter functions --- //


};
#endif //RESPONSEGENERATOR_DELETE_HPP
