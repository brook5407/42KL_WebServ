#include "LimitRequestBodyHandler.hpp"
#include "HttpException.hpp"

void LimitRequestBodyHandler::execute(Request &req, Response &res)
{
    if (req.get_location_config().getMaxBodySize())
    {
        if (req.get_body_length() > req.get_location_config().getMaxBodySize())
            throw HttpException(413, "Request Entity Too Large");
    }
    else if (req.get_body_length() > req.get_server_config().getMaxBodySize())
    {
        throw HttpException(413, "Request Entity Too Large");
    }
    Middleware::execute(req, res);
}