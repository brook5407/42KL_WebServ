#ifndef MIDDLEWARE_HPP
# define MIDDLEWARE_HPP

# include "Request.hpp"
# include "Response.hpp"

class Middleware
{
    public:
        Middleware();
        virtual ~Middleware() = 0;

        virtual void    execute(Request &req, Response &res);
        void            setNext(Middleware *next);
        Middleware      *getNext();

    protected:
        Middleware *_next;
};

#endif
