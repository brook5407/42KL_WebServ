#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "ConfigParser.hpp"
#include <stdlib.h>

int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
        {
            std::cout << "Usage: ./" << *argv << " <config_file>" << std::endl;
            return (1);
        }
        ConfigParser    parser(argv[1]);
        std::vector<Server>   servers = parser.getServers();
        for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
            std::cout << *it << std::endl;
        system("leaks -q a.out");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        system("leaks -q a.out");
    }
}
