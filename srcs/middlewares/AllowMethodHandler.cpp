#include "AllowMethodHandler.hpp"
#include "HttpException.hpp"

void AllowMethodHandler::execute(Request &req, Response &res)
{
    if (req.get_method() == "HEAD") // not supported method
        return res.send_content(405, std::string()); // no-body hack
    if (req.get_location_config().getMethods().count(req.get_method()))
        return Middleware::execute(req, res);
    throw HttpException(405, "Method Not Allowed");
}
