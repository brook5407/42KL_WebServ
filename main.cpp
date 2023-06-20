#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "ConfigParser.hpp"

int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
        {
            std::cout << "Usage: ./webserv <config_file>" << std::endl;
            return (1);
        }
        ConfigParser    parser(argv[1]);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
