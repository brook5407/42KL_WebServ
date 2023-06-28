#include "Webserver.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    const char *config_filepath = "conf/webserv.conf";
    try
    {
        if (argc == 2)
            config_filepath = argv[1];
        else if (argc > 2)
            throw std::invalid_argument("usage: webserv <conf_file>");
        Webserver webserver(config_filepath);
        webserver.loop();
        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}