#include "UploadHandler.hpp"
#include "HttpException.hpp"
#include "Util.hpp"
#include <iostream>

static std::string get_filename(const std::string &Content_disposition);
static void save_file(const std::string &filename, const std::string &content);
static bool checkMultiPart(const std::string &content_type);

// todo think about http response status code for different failures
void UploadHandler::execute(Request &req, Response &res)
{
    if (req.get_method() == "POST" && checkMultiPart(req.get_header("Content-Type")) )
    {
        //get boundary information for headers
        const std::string &content_type = req.get_header("Content-Type");
        std::string::size_type pos = content_type.find("boundary=");
        if (pos != std::string::npos)
        {
            std::string boundary = content_type.substr(pos + 9);
            std::string body = req.get_body();
            // remove last line boundary
            std::string::size_type pos_bd_end = body.find_last_of(boundary);
            body = body.substr(0, pos_bd_end - 4);
            std::string::size_type pos_end = body.find_last_of("\r\n");
            body = body.substr(0, pos_end - 1);
            //find last boundary
            std::string::size_type pos_bd = body.find(boundary);
            while (pos_bd != std::string::npos)
            {
                body = body.substr(pos_bd + boundary.size() + 2);
                pos_bd = body.find(boundary);
            }
            //find filename from body
            std::string::size_type pos_fn_start = body.find("filename=") + 10;
            std::string::size_type pos_fn_end = body.find("\r\n");
            std::string filename = body.substr(pos_fn_start, pos_fn_end - pos_fn_start);
            pos_fn_end = filename.find_last_of("\"");
            filename = filename.substr(0, pos_fn_end);
            // find emptyline
            std::string::size_type pos_emptyline = body.find("\r\n\r\n");
            body = body.substr(pos_emptyline + 4);
            if (filename.empty())
                throw HttpException(400, "Bad Request: filename not found");
            // save file
            filename = Util::combine_path(req.get_translated_path(), filename);
            std::cout << "filename: " << filename << " sz:" << body.size() << std::endl;
            std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
            if (!ofs.is_open())
                throw HttpException(500, "Internal Server Error: failed to open file");
            ofs << body;
            res.send_content(200, "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Upload Success</title></head><body> \
            <h1>Upload Successful</h1><p>Your file has been successfully uploaded.</p> \
            <p><a href=\"javascript:history.back()\">Go back</a></p></body></html>");
        }
        else
            throw HttpException(400, "Bad Request: boundary not found");
    }
    // for curl & tester, not for browser multipart/form-data
    else if ((req.get_method() == "POST" || req.get_method() == "PUT"))
    {
        //post /dir/<f> , /dir/filename/<f>,  w/wo:Content-Disposition
        std::string filename = get_filename(req.get_header("Content-Disposition"));
        if (filename.size())
            filename = Util::combine_path(req.get_translated_path(), filename); // prefix document root
        else
            filename = req.get_translated_path();
        save_file(filename, req.get_body());
        res.send_content(200, filename.substr(filename.find_last_of('/') + 1) + " has been uploaded!");
    }
    else if (req.get_method() == "DELETE")
    {
        std::string filename = req.get_translated_path();
        if (filename[filename.size() - 1] == '/')
            filename.erase(filename.size() - 1);
        if (remove(filename.c_str()) == 0)
            res.send_content(200, filename + " has been deleted!\n");
        else
            throw HttpException(400, "Bad Request: file not found");
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
    std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (ofs.fail())
        throw HttpException(500, "Internal Server Error: fail to open file");
    ofs << content;
}

bool checkMultiPart(const std::string &content_type)
{
    std::string::size_type pos = content_type.find("multipart/form-data");
    if (pos != std::string::npos)
        return true;
    return false;
}
