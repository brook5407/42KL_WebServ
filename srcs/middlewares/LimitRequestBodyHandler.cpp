#include "LimitRequestBodyHandler.hpp"
#include "HttpException.hpp"

void LimitRequestBodyHandler::execute(Request &req, Response &res)
{
    if (req._location_config->getMaxBodySize())
    {
        if (req._body.size() > req._location_config->getMaxBodySize())
            throw HttpException(413, "Request Entity Too Large");
    }
    else if (req._body.size() > req._server_config->getMaxBodySize())
    {
        throw HttpException(413, "Request Entity Too Large");
    }
    Middleware::execute(req, res);
}