#include "ErrorHandler.hpp"
#include "Singleton.hpp"
#include "HttpException.hpp"
#include "ReasonPhrase.hpp"
#include "Util.hpp"

static void generate_html(std::stringstream &ss, int status_code);

void ErrorHandler::execute(Request &req, Response &res)
{
    try
    {
        if (!req.get_location_config().checkRedirection() && req.get_translated_path().empty())
            throw HttpException(404, "no location matched");
        Middleware::execute(req, res);
    }
    catch (const HttpException &e)
    {
        send_error(req, res, e.status_code(), e.what());
    }
    catch (const std::exception &e)
    {
        send_error(req, res, 500, e.what());
    }
}

void ErrorHandler::send_error(Request &req, Response &res, int status_code, const char *)
{
    // std::cerr << "HttpException: " << what << std::endl;
    const std::string filepath = req.get_server_config().getErrorPagePath(status_code);
        if (filepath.size() && Util::file_exists(filepath))
        {
        res.send_error_file(status_code, filepath);
        }
        else
        {
            std::stringstream ss;
        generate_html(ss, status_code);
        res.send_content(status_code, ss.str());
    }
}

void generate_html(std::stringstream &ss, int status_code)
{
    const std::string reason = Singleton<ReasonPhrase>::get_instance().lookup(status_code);
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
