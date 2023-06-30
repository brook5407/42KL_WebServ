#include "AutoIndexHandler.hpp"
#include <dirent.h>

static void generate_html(std::stringstream &ss, DIR *dir, const std::string &uri);
static void upload_form(std::stringstream &ss);
static void show_toggle(std::stringstream &ss);

void AutoIndexHandler::execute(Request &req, Response &res)
{
    DIR *dir;

    if (req._location_config->checkAutoIndex() == false)
        return Middleware::execute(req, res);
    dir = opendir(req._script_name.c_str());
    if (dir == NULL)
        return Middleware::execute(req, res);
    std::stringstream ss;
    generate_html(ss, dir, req._uri);
    closedir(dir);
    res.send_content(200, ss.str());
}

void generate_html(std::stringstream &ss, DIR *dir, const std::string &uri)
{
    struct dirent *entry;

    ss << "<html><body>";
    ss << "<h1>Index of " + uri + "</h1>";
    ss << "<pre>";
    ss << "<img src=\"/.hidden/icon/blank.png\"";
    ss << "<a href=\"?NA\">Name</a>";
    ss << "<hr>";
    if (uri != "/")
    {
        ss << "<img src=\"/.hidden/icon/up.png\">";
        ss << "<a href=\"..\">Parent Directory</a>";
    }

    while ((entry = readdir(dir)) != NULL)
    {
        // hide dot files
        if (entry->d_name[0] == '.')
            continue;
        if (entry->d_type != DT_DIR && entry->d_type != DT_REG)
            continue;
        if (entry->d_type == DT_DIR)
            ss << "<li><img src=\"/.hidden/icon/folder.png\"    >";
        else
            ss << "<li><img src=\"/.hidden/icon/image.png\" >";
        ss << std::string() + "<a href=\"" + uri;
        // add curent directory slash if not present
        if (uri[uri.size() - 1] != '/')
            ss << "/";
        ss << std::string() + entry->d_name;
        // append slash to subdirectory
        if (entry->d_type == DT_DIR)
            ss << "/";
        ss << std::string() + "\">" + entry->d_name;
        if (entry->d_type == DT_DIR)
            ss << "/";
        ss << "</a>";
    }

    ss << "</li></pre><hr>";
    ss << "<input type=\"button\" value=\"Upload\" onclick=\"show()\">";
    upload_form(ss);
    show_toggle(ss);
    ss << "</body></html>";
}

void upload_form(std::stringstream &ss)
{
    ss <<
        "<p>"
        "<form hidden method=\"post\" enctype=\"multipart/form-data\" id=\"fileToUpload\">"
        "<label for=\"fileToUpload\">Select a file:</label> "
        "<input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\">"
        "<input type=\"submit\" id=\"fileToUpload\">"
        "</form>"
        "</p>";
}

void show_toggle(std::stringstream &ss)
{
    ss <<
        "<script>"
        "function show() {"
            "var x = document.getElementById(\"fileToUpload\");"
            "if (x.style.display === \"block\") {"
                "x.style.display = \"none\";"
            "} else {"
                "x.style.display = \"block\";"
            "}"
        "}"
        // hide broken images
        "document.addEventListener('DOMContentLoaded', function() {"
            "document.querySelectorAll('img').forEach(function(el, i){"
                "el.addEventListener('error', function(){"
                    "el.style.display = 'none';"
                "});"
            "});"
        "});"
        "</script>";
}
