#ifndef INDEXHANDLER_HPP
# define INDEXHANDLER_HPP

# include "Middleware.hpp"

class IndexHandler : public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
