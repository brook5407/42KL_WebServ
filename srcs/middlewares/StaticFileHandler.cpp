#include "StaticFileHandler.hpp"

void StaticFileHandler::execute(Request &req, Response &res)
{
    if (file_exists(req._script_name))
        res.send_file(200, req._script_name);
    else
        throw HttpException(404, "Not found");
}
