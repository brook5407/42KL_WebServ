#include "AllowMethodHandler.hpp"

void AllowMethodHandler::execute(Request &req, Response &res)
{
    if (req._method == "HEAD") // not supported method
        return res.send_content(405, std::string()); // no-body hack
    if (req._location_config->getMethods().count(req._method))
        return Middleware::execute(req, res);
    throw HttpException(405, "Method Not Allowed");
}
