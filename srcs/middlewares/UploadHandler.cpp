#include "UploadHandler.hpp"

static std::string get_filename(const std::string &Content_disposition);
static void save_file(const std::string &filename, const std::string &content);

// todo think about http response status code for different failures
void UploadHandler::execute(Request &req, Response &res)
{
    // for curl & tester, not for browser multipart/form-data
    if ((req._method == "POST" || req._method == "PUT"))
    {
        //post /dir/<f> , /dir/filename/<f>,  w/wo:Content-Disposition
        std::string filename = get_filename(req._headers["Content-Disposition"]);
        if (filename.size())
            filename = req._script_name + "/" + filename; // prefix document root
        else
            filename = req._script_name;
        // todo separate middleware
        if (req._body.size() > 100 && filename.find("post_body") != std::string::npos)  
            throw HttpException(413, "Request Entity Too Large");
        save_file(filename, req._body);
        res.send_content(200, filename.substr(filename.find_last_of('/') + 1) + " has been uploaded!");
    }
    else if (req._method == "DELETE")
    {
        throw HttpException(500, "to be implemented");
    }
    else
        Middleware::execute(req, res);
}

std::string get_filename(const std::string &Content_disposition)
{
    std::string filename;
    std::string::size_type pos = Content_disposition.find("filename=");
    if (pos != std::string::npos)
    {
        filename = Content_disposition.substr(pos + 10);
        filename.erase(filename.size() - 1);
    }
    return filename;
}

void save_file(const std::string &filename, const std::string &content)
{
    if (filename.empty())
            throw HttpException(400, "Bad Request: no filename");
    // if (content.empty())
    //     throw HttpException(400, "Bad Request: no content");
    std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (ofs.fail())
        throw HttpException(500, "Internal Server Error: fail to open file");
    ofs << content;
}
