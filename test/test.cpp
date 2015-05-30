#include "../Jzon.h"

#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Expecting 1 argument - a file name" << std::endl;
		return 1;
	}

	std::string filename(argv[1]);
	
	Jzon::Parser parser;
	
	Jzon::Node node = parser.parseFile(filename);
	if (!node.isValid())
	{
		std::cerr << parser.getError() << std::endl;
		return 1;
	}

	return 0;
}
