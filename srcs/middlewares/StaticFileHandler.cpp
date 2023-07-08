#include "StaticFileHandler.hpp"
#include "HttpException.hpp"
#include "Util.hpp"

void StaticFileHandler::execute(Request &req, Response &res)
{
    if (Util::file_exists(req._script_name))
        res.send_file(200, req._script_name);
    else
        throw HttpException(404, "Not found");
}
