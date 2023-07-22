#include "CheckMethodHandler.hpp"
#include "HttpException.hpp"

void CheckMethodHandler::execute(Request &req, Response &res)
{
    if (req.get_location_config().getMethods().count(req.get_method())) // check if method exists
        return Middleware::execute(req, res);
    throw HttpException(405, "Method Not Allowed");
}
