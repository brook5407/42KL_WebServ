#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <map>
#include <vector>
#include <fcntl.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <string>
#include <sstream>
#include <stdexcept>

#include "Connection.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Middleware.hpp"
#include "Webserv.hpp"


int main(int argc, char **argv, char **)
{
signal(SIGPIPE, SIG_IGN);
        if (argc != 2)
            throw std::runtime_error("usage: webserv <port>");
        int port = atoi(argv[1]);
        Webserv webserv;
        webserv.init();
        webserv.start(port);
        return EXIT_SUCCESS;
    try
    {
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
