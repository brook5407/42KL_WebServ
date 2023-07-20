#ifndef CHECKMETHODHANDLER_HPP
# define CHECKMETHODHANDLER_HPP

# include "Middleware.hpp"

class CheckMethodHandler : public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
