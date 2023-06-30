#include "StaticFileHandler.hpp"

void StaticFileHandler::execute(Request &req, Response &res)
{
    struct stat sb;
    if (stat(req._script_name.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
        res.send_file(200, req._script_name);
    else
        throw HttpException(404, "Not found");
}