#include "ErrorHandler.hpp"

void ErrorHandler::execute(Request &req, Response &res)
{
    try
    {
        if (!req._location_config->checkRedirection() && req._script_name.empty())
            throw HttpException(404, "no location matched");
        Middleware::execute(req, res);
    }
    catch (const HttpException &e)
    {
        std::cout << "HTTP exception: " << e.what() << std::endl;
        std::string error_page = "./error_pages/" + to_string(e.status_code()) + ".html";
        std::cout << "error page: " << error_page << " for " << req._uri << std::endl;
        res.send_file(e.status_code(), error_page);
    }
}
