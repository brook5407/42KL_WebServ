#ifndef ERRORHANDLER_HPP
# define ERRORHANDLER_HPP

# include "Middleware.hpp"

// todo must check error file exist
class ErrorHandler: public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
