#ifndef LIMITREQUESTBODYHANDLER_HPP
# define LIMITREQUESTBODYHANDLER_HPP

#include "Middleware.hpp"

class LimitRequestBodyHandler : public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
