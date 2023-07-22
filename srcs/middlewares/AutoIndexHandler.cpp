#include "AutoIndexHandler.hpp"
#include "HttpException.hpp"
#include "Util.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>

static void generate_html(std::stringstream &ss, DIR *dir, const std::string &uri, const std::string &path);
static void css_form(std::stringstream &ss);
static void upload_form(std::stringstream &ss);
static void show_toggle(std::stringstream &ss);

void AutoIndexHandler::execute(Request &req, Response &res)
{
    if (!Util::dir_exists(req.get_translated_path()))
        return Middleware::execute(req, res);
    if (req.get_location_config().checkAutoIndex() == false)
    {
        // if (req.get_method() == "GET")
        //     throw HttpException(403, "listing denied");
        return Middleware::execute(req, res);
    }
    DIR *dir = opendir(req.get_translated_path().c_str());
    std::stringstream ss;
    generate_html(ss, dir, req.get_uri(), req.get_translated_path());
    closedir(dir);
    res.send_content(200, ss.str());
}

void generate_html(std::stringstream &ss, DIR *dir, const std::string &uri, const std::string &path)
{
    struct dirent *entry;
    struct stat st;
    char datetime[80];

    css_form(ss);
    ss << "<html><body>";
    ss << "<h1>Index of " << uri << "</h1>";
    ss << "<ul><table>";
    ss << "<tr><th><img src=\"/.hidden/icon/blank.png\" alt=\" \"> ";
    ss << "Name</th>";
    ss << "<th>Size</a></th>";
    ss << "<th>Last Modified</th>";
    ss << "</tr>";
    // show parent directory if not root
    if (uri != "/")
    {
        ss << "<tr>";
        ss << "<td><img src=\"/.hidden/icon/up.png\"> ";
        ss << "<a href=\"..\">Parent Directory</a></td>";
        ss << "<td>-</td>";
        ss << "<td>-</td>";
        ss << "</tr>";
    }
    ss << "<tr>";
    while ((entry = readdir(dir)) != NULL)
    {
        // hide dot files
        if (entry->d_name[0] == '.')
            continue;
        if (entry->d_type != DT_DIR && entry->d_type != DT_REG)
            continue;
        if (entry->d_type == DT_DIR)
            ss << "<td><img src=\"/.hidden/icon/folder.png\"> ";
        else
            ss << "<td><img src=\"/.hidden/icon/image.png\"> ";
        ss << "<a href=\"" << uri;
        // add curent directory slash if not present
        if (uri[uri.size() - 1] != '/')
            ss << "/";
        ss << std::string() + entry->d_name;
        // append slash to subdirectory
        if (entry->d_type == DT_DIR)
            ss << "/";
        ss << "\">" << entry->d_name;
        if (entry->d_type == DT_DIR)
            ss << "/";
        ss << "</a></td>";
        ss << "<td>";
        // show the size of the file or folder
        std::string filepath = path + "/" + entry->d_name;
        stat(filepath.c_str(), &st);
        if (entry->d_type == DT_REG)
        {
            if (st.st_size > 1000000)
                ss << st.st_size / 1000000 << " MB";
            else if (st.st_size > 1000)
                ss << st.st_size / 1000 << " KB";
            else
                ss << st.st_size << " Bytes";
        }
        else
            ss << "-";
        ss << "</td>";
        // show the last modified date
        std::strftime(datetime, sizeof(datetime), "%d-%b-%Y %H:%M", std::localtime(&st.st_ctime));
        ss << "<td>" << datetime << "</td>";
        ss << "</tr>";
    }
    ss << "</table></ul>";
    ss << "<input type=\"button\" value=\"Upload\" onclick=\"show()\">";
    upload_form(ss);
    show_toggle(ss);
    ss << "</body></html>";
}

void css_form(std::stringstream &ss)
{
    ss << "<head>"
    "<title>Directory Listing</title>"
    "<style>"
    "ul {"
    "    list-style-type: none;"
    "    padding: 0;"
    "}"

    "table {"
    "    width: 100%;"
    "    table-layout: fixed;"
    "}"

    "th, td {"
    "    white-space: nowrap;"
    "    overflow: hidden;"
    "    text-overflow: ellipsis;"
    "    font-size: 14px;"
    "    padding-right: 40px;"
    "}"

    "th {"
    "    text-align: left;"
    "    border-bottom: 1px solid #000;"
    "}"

    "td {"
    "    border-bottom: 1px solid #ddd;"
    "}"

    "td:first-child {"
    "    width: 70%;"
    "}"

    "td:last-child {"
    "    width: 20%;"
    "    white-space: nowrap;"
    "    padding-right: 0;"
    "}"

    "img {"
    "    vertical-align: middle;"
    "}"
    "</style>"
    "</head>";
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
