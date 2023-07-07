#include "ErrorHandler.hpp"
#include "Singleton.hpp"
#include "HttpException.hpp"
#include "ReasonPhrase.hpp"

static void generate_html(std::stringstream &ss, int status_code, std::string reason);

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
        const std::string filepath = req._server_config->getErrorPagePath(e.status_code());
        if (filepath.size() && file_exists(filepath))
        {
            res.send_file(e.status_code(), filepath);
        }
        else
        {
            std::string reason = Singleton<ReasonPhrase>::get_instance().lookup(e.status_code());
            std::stringstream ss;
            generate_html(ss, e.status_code(), reason);
            res.send_content(e.status_code(), ss.str());
        }
    }
}

void generate_html(std::stringstream &ss, int status_code, std::string reason)
{
    ss << \
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>" << status_code << "</title>"
        "<style>"
            "body {"
            "font-family: Arial, sans-serif;"
            "text-align: center;"
            //   "padding: 50px;"
            "}"
            "h1 {"
            "font-size: 48px;"
            "margin-bottom: 20px;"
            "}"
            "p {"
            "font-size: 24px;"
            "margin-bottom: 30px;"
            "}"
        "</style>"
        "</head>"
        "<body>"
        "<h1>" << status_code << ' ' << reason << "</h1>"
        "<hr/><p>webserv</p>"
        "</body>"
        "</html>";
}
