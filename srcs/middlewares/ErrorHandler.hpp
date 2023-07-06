#ifndef ERRORHANDLER_HPP
# define ERRORHANDLER_HPP

# include "Middleware.hpp"

class ErrorHandler: public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
