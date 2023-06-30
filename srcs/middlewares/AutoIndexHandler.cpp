#include "AutoIndexHandler.hpp"
#include <dirent.h>

void AutoIndexHandler::execute(Request &req, Response &res)
{
    DIR *dir;
    struct dirent *entry;

    if (req._location_config->checkAutoIndex() == false)
        return Middleware::execute(req, res);
    dir = opendir(req._script_name.c_str());
    if (dir == NULL)
        return Middleware::execute(req, res);
    std::stringstream ss;
    ss << "<html><body>";
    ss << "<h1>Directory listing</h1>";
    ss << "<ol>";

    while ((entry = readdir(dir)) != NULL)
    {
        // hide dot files
        if (entry->d_name[0] == '.')
            continue;
        if (entry->d_type != DT_DIR && entry->d_type != DT_REG)
            continue;
        ss << std::string() + "<li><a href=\"" + req._uri;
        // add directory slash if not present
        if (req._uri[req._uri.size() - 1] != '/')
            ss << "/";
        ss << std::string() + entry->d_name;
        // append slash to directory
        if (entry->d_type == DT_DIR)
            ss << "/";
        ss << std::string() + "\">" + entry->d_name;
        if (entry->d_type == DT_DIR)
            ss << "/";
        ss << "</a></li>";
    }

    closedir(dir);
    ss << "</ol>";
    ss << "</html></body>";
    res.send_content(200, ss.str());
}
