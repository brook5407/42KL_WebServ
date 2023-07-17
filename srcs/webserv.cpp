#include "Webserver.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char **argv)
{
    int cgi_timeout = 5;
    const char *config_filepath = "conf/webserv.conf";
    try
    {
        if (argc >= 2)
            config_filepath = argv[1];
        if (argc == 3)
        {
            cgi_timeout = std::atoi(argv[2]);
            if (cgi_timeout < 1 || cgi_timeout > 3600)
                throw std::out_of_range("Invalid CGI timeout value (1 - 3600)");
        }
        else if (argc > 3)
            throw std::invalid_argument("usage: webserv <conf_file>");
        srand(time(NULL));
        Webserver webserver(config_filepath, cgi_timeout);
        webserver.loop();
        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
