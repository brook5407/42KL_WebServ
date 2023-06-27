// #include <unistd.h>
// #include <netinet/in.h>
// #include <sys/socket.h>
#include <iostream>
// #include <map>
// #include <vector>
// #include <fcntl.h>

// #include <cerrno>
// #include <cstdio>
// #include <cstdlib>
// #include <cstring>
// #include <sys/select.h>
// #include <string>
// #include <sstream>
// #include <stdexcept>

// #include "Configuration.hpp"
// #include "Connection.hpp"
// #include "Request.hpp"
// #include "Response.hpp"
// #include "Middleware.hpp"
#include "Webserver.hpp"

int main(int argc, char **argv, char **)
{
    try
    {
        if (argc != 2)
            throw std::runtime_error("usage: webserv <conf_file>");      
        char *config_filepath = argv[1];
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
