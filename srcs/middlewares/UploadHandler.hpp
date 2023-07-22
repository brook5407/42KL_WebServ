#ifndef UPLOADHANDLER_HPP
# define UPLOADHANDLER_HPP

# include "Middleware.hpp"

class UploadHandler: public Middleware
{
    public:
        void execute(Request &req, Response &res);
};

#endif
