#include <unistd.h>
#include "Parser.hpp"
#include "colors.h"

int	main(int argc, char *argv[])
{
	//config parser
	Parser parser;//constructor with default.conf
	if (argc > 1)
		parser = Parser(argv[1]);//If there is an argument, make a copy
	if (parser.parse())
		std::cout << "Yuppiii!!!!" << std::endl;
	else
		std::cout << RED "Parsing Failed!!!!" RESET << std::endl;
	//set sockets

	// while (1)
	// {
	// 	//select() or smt equivalent
	// }
	// sleep (5);
	return 0;
}
