#include "RedirectHandler.hpp"

void RedirectHandler::execute(Request &req, Response &res)
{
    if (req._location_config->getRedirection().first)
    {
        res.send_location(
            req._location_config->getRedirection().first,
            req._location_config->getRedirection().second);
    }
    else
        Middleware::execute(req, res);
}
