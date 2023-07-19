#include "RedirectHandler.hpp"
#include <dirent.h>

void RedirectHandler::execute(Request &req, Response &res)
{
    if (req.get_location_config().checkRedirection())
    {
        res.send_location(
            req.get_location_config().getRedirection().first,
            req.get_location_config().getRedirection().second);
    }
    else
    {
        // if (req.get_method() == "GET" 
        //     && req.get_uri().size() > 1
        //     && *req.get_uri().rbegin() != '/')
        // {
        //     DIR *dir = opendir(req.get_translated_path().c_str());
        //     if (dir)
        //     {
        //         closedir(dir);
        //         res.send_location(301, req.get_uri() + "/");
        //         return;
        //     }
        // }
        Middleware::execute(req, res);
    }
}
