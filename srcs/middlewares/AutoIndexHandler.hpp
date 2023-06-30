#ifndef AUTOINDEXHANDLER_HPP
# define AUTOINDEXHANDLER_HPP

# include "Middleware.hpp"

class AutoIndexHandler: public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
