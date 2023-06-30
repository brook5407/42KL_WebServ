#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "Middleware.hpp"
# include "CGI.hpp"
#include <list>

class CGIHandler: public Middleware
{
    public:
        std::list<CGI>  _CGI;
        void execute(Request &req, Response &res);
};

#endif
