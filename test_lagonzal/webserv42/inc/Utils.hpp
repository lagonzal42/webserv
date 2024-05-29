#ifndef UTILS_HPP
# define UTILS_HPP

// This is a Util class to put functions in general

# include <iostream>
# include <string>
# include <sstream>


class Utils
{
	public:
			Utils( void );
			~Utils( void );
			Utils( Utils const & src );
			Utils & operator=( Utils const & src );
	// --- Getter functions --- //
			static std::string	extractNumbers(std::string const & str);
			static int	obtainIntFromStr(std::string const & num);

	// --- Getter functions --- //




};
#endif //UTILS_HPP
