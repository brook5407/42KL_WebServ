#include "RedirectHandler.hpp"

void RedirectHandler::execute(Request &req, Response &res)
{
    if (req.get_location_config().checkRedirection())
    {
        res.send_location(
            req.get_location_config().getRedirection().first,
            req.get_location_config().getRedirection().second);
    }
    else
        Middleware::execute(req, res);
}
