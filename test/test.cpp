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
	Jzon::FileReader reader(filename);
	const std::string &error = reader.GetError();
	if (!error.empty())
	{
		std::cerr << error << std::endl;
		return 1;
	}

	Jzon::Node *node = NULL;
	switch (reader.DetermineType())
	{
	case Jzon::Node::T_ARRAY:
		node = new Jzon::Array;
		break;
	case Jzon::Node::T_OBJECT:
		node = new Jzon::Object;
		break;
	case Jzon::Node::T_VALUE:
		node = new Jzon::Value;
		break;
	default:
		std::cerr << "Sanity check fail" << std::endl;
		return 1;
	}

	bool result = reader.Read(*node);
	if (!result)
	{
		std::cerr << reader.GetError() << std::endl;
		return 1;
	}

	return 0;
}
