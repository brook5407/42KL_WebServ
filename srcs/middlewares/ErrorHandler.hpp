#ifndef ERRORHANDLER_HPP
# define ERRORHANDLER_HPP

# include "Middleware.hpp"

class ErrorHandler: public Middleware
{
    public:
        void        execute(Request &req, Response &res);
        static void send_error(
            Request &req, Response &res,
            int status_code, const char *what);
};

#endif
