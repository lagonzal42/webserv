#include <unistd.h>
#include "Parser.hpp"
#include "colors.h"
#include <stdexcept>

int	main(int argc, char *argv[])
{
	//config parser
	Parser parser;//constructor with default.conf
	if (argc > 1)
		parser = Parser(argv[1]);//If there is an argument, make a copy
	try
	{
		parser.parse();
	}
	catch (const std::exception & e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return 1;
	}
	// if (parser.parse())
	// 	std::cout << "Kaixo!" << std::endl;
	// else
	// 	std::cout << RED "Parsing Failed!!!!" RESET << std::endl;
	std::cout << "======>>> print test <<<======" << std::endl;
	std::cout << parser << RESET << std::endl;
	
	//set sockets

	// while (1)
	// {
	// 	//select() or smt equivalent
	// }
	// sleep (5);
	return 0;
}
