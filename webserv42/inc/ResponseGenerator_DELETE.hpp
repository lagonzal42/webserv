#ifndef RESPONSEGENERATOR_DELETE_HPP
# define RESPONSEGENERATOR_DELETE_HPP

// This is a ResponseGenerator class for DELETE request

# include <iostream>
# include <string>
# include <sstream>
# include "Request.hpp"
# include "Parser.hpp"
# include "colors.h"

/*=========HTTP CODES=========*/

#ifndef OK
# define OK 200
#endif
#ifndef NO_CONTENT
# define NO_CONTENT	204
#endif
#ifndef PERMANENT_REDIRECT
# define PERMANENT_REDIRECT	308
#endif
#ifndef BAD_REQUEST
# define BAD_REQUEST 400
#endif
#ifndef FORBIDEN
# define FORBIDEN 403
#endif
#ifndef NOT_FOUND
# define NOT_FOUND 404
#endif
#ifndef METHOD_NOT_ALLOWED
# define METHOD_NOT_ALLOWED 405
#endif
#ifndef TIMEOUT
# define TIMEOUT 408
#endif
#ifndef INTERNAL_SERVER_ERROR
# define INTERNAL_SERVER_ERROR 500
#endif

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1024
#endif

#ifndef NOT_IMPLEMENTED
# define NOT_IMPLEMENTED "Not implemented ^_^"
#endif


class ResponseGeneratorDELETE
{
	public:
			ResponseGeneratorDELETE( void );
			~ResponseGeneratorDELETE( void );
			ResponseGeneratorDELETE( ResponseGeneratorDELETE const & src );
			ResponseGeneratorDELETE & operator=( ResponseGeneratorDELETE const & src );
	// --- Getter functions --- //
			// I don't know why does work well with std::string, and not with char *
			static std::string generateDeleteResponse(Request & req, const Parser::Location & currentLoc, const Parser::Server & currentSer, const std::string &fullPath);
			static std::string generateHttpResponse(const std::string &status, const std::string &title, const std::string &message);

			// static const char	* generateDeleteResponse(Request & req, const Parser::Location & currentLoc, const std::string &fullPath);
	// --- Getter functions --- //


};
#endif //RESPONSEGENERATOR_DELETE_HPP
