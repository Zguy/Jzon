#include <iostream>
#include "../Jzon.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "Expecting 1 argument - a file name" << std::endl;
		return 1;
	}
	std::string fname(argv[1]);
	Jzon::FileReader readr(fname);
	std::string const err = readr.GetError();
	if (!err.empty()) {
		std::cerr << err << std::endl;
		return 1;
	}
	Jzon::Node *node;
	switch (readr.DetermineType())
	{
	case Jzon::Node::Type::T_ARRAY:
		node = new Jzon::Array;
		// std::cout << "It's an array!" << std::endl;
		break;
	case Jzon::Node::Type::T_OBJECT:
		node = new Jzon::Object;
		// std::cout << "It's an object!" << std::endl;
		break;
	case Jzon::Node::Type::T_VALUE:
		node = new Jzon::Value;
		// std::cout << "It's a value!" << std::endl;
		break;
	default:
		std::cerr << "Sanity check fail" << std::endl;
		return 1;
	}
	bool res = readr.Read(*node);
	if (!res) {
		std::cerr << readr.GetError() << std::endl;
		return 1;
	}
	// std::cout << "It worked!" << std::endl;
	return 0;
}