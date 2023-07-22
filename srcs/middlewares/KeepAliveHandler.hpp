#ifndef KEEPALIVEHANDLER_HPP
# define KEEPALIVEHANDLER_HPP

# include "Middleware.hpp"

class KeepAliveHandler: public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
