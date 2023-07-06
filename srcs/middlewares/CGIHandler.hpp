#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "Middleware.hpp"
# include "CGI.hpp"
#include <list>

class CGIHandler: public Middleware
{
    private:
        std::list<CGI> _CGI;

    public:
        void execute(Request &req, Response &res);
        void timeout(size_t execution_timeout_sec);
        void handle_exit(pid_t pid, int status);

};

#endif
