#include "Middleware.hpp"

Middleware::Middleware(): _next(NULL)
{}

Middleware::~Middleware()
{}

void Middleware::execute(Request &req, Response &res)
{
    if (_next)
        _next->execute(req, res);
}

void Middleware::setNext(Middleware *next)
{
    _next = next;
}

Middleware *Middleware::getNext()
{
    return _next;
}
