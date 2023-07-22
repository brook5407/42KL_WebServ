#ifndef STATICFILEHANDLER_HPP
# define STATICFILEHANDLER_HPP

# include "Middleware.hpp"

class StaticFileHandler: public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
