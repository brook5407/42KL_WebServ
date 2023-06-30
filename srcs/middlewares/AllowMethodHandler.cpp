#include "AllowMethodHandler.hpp"

void AllowMethodHandler::execute(Request &req, Response &res)
{
    if (req._method == "HEAD")
    {
        // reply 405 status code but without content
        res.send_content(405, std::string());
        return;
    }
    if (req._location_config->getMethods().empty())
        std::cout << "empty method" << std::endl;
    if (req._location_config->getMethods().count(req._method))
        return Middleware::execute(req, res);
    throw HttpException(405, "Method Not Allowed");
}