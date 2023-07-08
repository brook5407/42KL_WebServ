#ifndef MIDDLEWARE_HPP
#define MIDDLEWARE_HPP

#include "Request.hpp"
#include "Response.hpp"

class Middleware
{
    public:
        Middleware();
        virtual ~Middleware() = 0;

        virtual void execute(Request &req, Response &res);
        void setNext(Middleware *next);
        Middleware *getNext();

    protected:
        Middleware *_next;
};

class Logger: public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            // if (req._body.size())
            // {
            //     std::ofstream ofs("body.log", std::ios::out | std::ios::trunc | std::ios::binary);
            //     ofs << req._body;   
            // }
            Middleware::execute(req, res);
        }
};

class KeepAliveHandler: public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            res.set_keep_alive(
                !req._headers.count("Connection")
                || req._headers["Connection"] != "close");
            Middleware::execute(req, res);
        }
};

#endif
