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

    if (filename == "escaped_chars.json")
    {        
        Jzon::Node node = Jzon::object();
        node.add("test_string", "bs:\\ fs:/ dq:\" nl:\n1 tb:\t2 bs:\b3 ff:\f4 cr:\r5"); 

        // NOTE: if compiled as a Windows console app, \f is displayed as the Venus Symbol character
        std::cout << node.get("test_string").toString() << std::endl;

        Jzon::Writer writer(Jzon::NoFormat);
        writer.writeFile(node, filename);
    }
	
	Jzon::Parser parser;
	
	Jzon::Node node = parser.parseFile(filename);
	if (!node.isValid())
	{
		std::cerr << parser.getError() << std::endl;
		return 1;
	}

    if (filename == "escaped_chars.json")
    {
        std::cout << node.get("test_string").toString() << std::endl;
        system("pause");
    }

	return 0;
}
