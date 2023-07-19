#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "Middleware.hpp"
# include "CGI.hpp"
# include <list>

class CGIHandler: public Middleware
{
    private:
        static std::list<CGI> _cgi_processes;

    public:
        void execute(Request &req, Response &res);
        static void timeout(size_t execution_timeout_sec);
        static void handle_exit(void);

};

#endif
