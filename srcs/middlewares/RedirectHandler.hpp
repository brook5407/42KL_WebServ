#ifndef REDIRECTHANDLER_HPP
# define REDIRECTHANDLER_HPP

# include "Middleware.hpp"

class RedirectHandler: public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
