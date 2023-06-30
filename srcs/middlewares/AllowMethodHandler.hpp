#ifndef ALLOWMETHODHANDLER_HPP
# define ALLOWMETHODHANDLER_HPP

# include "Middleware.hpp"

class AllowMethodHandler : public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
