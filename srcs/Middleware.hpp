#ifndef MIDDLEWARE_HPP
#define MIDDLEWARE_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "HttpException.hpp"

#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include "CGI.hpp"
#include "Cookie.hpp"

template <typename T>
struct Singleton
{
    static T *get_instance()
    {
        static T instance;
        return &instance;
    }
};

//abstract class
class Middleware
{
    public:
        Middleware(): _next(NULL) {};
        virtual ~Middleware() {};

        virtual void execute(Request &req, Response &res)
        {
            if (_next)
                _next->execute(req, res);
        };

        void setNext(Middleware *next)
        {
            _next = next;
        }

        Middleware *getNext()
        {
            return _next;
        }

    protected:
        Middleware *_next;

        // add check file function here
        bool	file_exists(std::string &filepath)
        {
	        std::ifstream	file(filepath.c_str());
	        return file.good();
        }

        bool   file_executable(std::string &filepath)
        {
            return (access(filepath.c_str(), X_OK) == 0);
        }

        bool    file_extension(const std::string &filepath, const std::string &extension)
        {
            return (filepath.size() > extension.size()
                && filepath.find(extension, filepath.size() - extension.size()) != std::string::npos);
            // return (filepath.substr(filepath.find_last_of('.')) == extension);
        }

        template <typename T>
        std::string to_string(T value)
        {
            std::ostringstream os;
            os << value;
            return os.str();
        }

};

class CheckMethod : public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            // not supported method
            if (req._method == "HEAD")
            {
                res.send_content(405, std::string());
                return;
            }
            if (req._location_config->getMethods().empty())
                std::cout << "empty method" << std::endl;
            if (req._location_config->getMethods().count(req._method))
                return Middleware::execute(req, res);
            throw HttpException(405, "Method Not Allowed");
        }
};

class CheckBodySize : public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            if (req._body.size() > req._server_config->getMaxBodySize())
                throw HttpException(413, "Request Entity Too Large");
            Middleware::execute(req, res);
        }
};

class IndexFile : public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            for (std::size_t i = 0; i < req._location_config->getIndex().size(); ++i)
            {
                const std::string& filename = req._location_config->getIndex()[i];
                DIR *dir = opendir(req._script_name.c_str());
                if (dir)
                {
                    closedir(dir);

                    std::string index_file = req._script_name;
                    if (req._script_name[req._script_name.size() - 1] != '/')
                        index_file += "/";
                    index_file += filename;
                    std::ifstream infile(index_file.c_str());
                    if (infile.is_open())
                        req._script_name = index_file;
                    // std::cout << "test index " << index_file << " scrpt:" << req._script_name << std::endl;
                }
            }
            Middleware::execute(req, res);
        }
};

class DirectoryListing: public Middleware
{
    public:
        void execute(Request &req, Response &res)
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
            ss << "<h1>Index of " + req._uri + "</h1>";
            ss << "<pre>";
            ss << "<img src=\"/.hidden/icon/blank.png\"";
            ss << "<a href=\"?NA\">Name</a>";
            ss << "<hr>";
            if (req._uri != "/")
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
                ss << std::string() + "<a href=\"" + req._uri;
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
                ss << "</a>";
            }

            closedir(dir);
            ss << "</li></pre><hr>";
            ss << "<input type=\"button\" value=\"Upload\" onclick=\"show()\">";
            upload(ss);
            show(ss);
            ss << "</html></body>";
            res.send_content(200, ss.str());
        }
        private:
            void upload(std::stringstream &ss)
            {
                ss << "<p>";
                ss << "<form hidden method=\"post\" enctype=\"multipart/form-data\" id=\"fileToUpload\">";
                ss << "<label for=\"fileToUpload\">Select a file:</label> ";
                ss << "<input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\">";
                ss << "<input type=\"submit\" id=\"fileToUpload\">";
                ss << "</form>";
                ss << "</p>";
            }
            void show(std::stringstream &ss)
            {
                ss << "<script>";
                ss << "function show() {";
                ss << "var x = document.getElementById(\"fileToUpload\");";
                ss << "if (x.style.display === \"block\") {";
                ss << "x.style.display = \"none\";";
                ss << "} else {";
                ss << "x.style.display = \"block\";";
                ss << "}";
                ss << "}";
                ss << "</script>";
            }
};

class Redirect: public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            if (req._location_config->getRedirection().first)
            {
                res.send_location(
                    req._location_config->getRedirection().first,
                    req._location_config->getRedirection().second);
            }
            else
                Middleware::execute(req, res);
        }
};

class StaticFile: public Middleware
{
    public:
        //todo test permission
        void execute(Request &req, Response &res)
        {
            struct stat sb;
            if (stat(req._script_name.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
                res.send_file(200, req._script_name);
            else
                throw HttpException(404, "Not found");
        }
};

// todo must check error file exist
class ErrorPage: public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            try
            {
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
};

class SessionHandler : public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            current_session_ID = extract_cookie(req._headers["Cookie"], "ID");
            if (current_session_ID.empty())
            {
                current_session_ID = to_string(rand());
                res.set_header("Set-Cookie", "ID=" + current_session_ID);
            }
            Middleware::execute(req, res);
        }

        const std::string &get_session()
        {
            return _session[current_session_ID];
        }

        const std::string &get_session_id() const
        {
            return current_session_ID;
        }

        std::string extract_cookie(const std::string &cookies, const std::string &cookie_name)
        {
            std::size_t begin = cookies.find(cookie_name + "=");
            if (begin == std::string::npos)
                return std::string();
            begin =+ cookie_name.size() + 1;
            std::size_t end = cookies.find(';', begin);
            const std::string cookie_value = cookies.substr(begin, end - begin);
            return cookie_value;
        }

        void set_session(const std::string &session_id, const std::string &new_value)
        {
            _session[session_id] = new_value;
        }


        std::string current_session_ID;
        std::map<std::string, std::string> _session;
};

extern char **environ;

class CgiRunner: public Middleware
{
    public:
        std::list<CGI>  _CGI;
        bool    is_CGI;
        void execute(Request &req, Response &res)
        {
            is_CGI = false;
            for (std::size_t i = 0; i < req._location_config->checkCgiExtension().size(); ++i)
            {
                const std::string &extension = req._location_config->checkCgiExtension()[i];
                if (file_extension(req._script_name, extension))
                {
                    // if (!file_exists(req._script_name))
                    //     throw HttpException(404, "File not found");
                    // if (!file_executable(req._script_name))
                    //     throw HttpException(403, "File is not executable");
                    is_CGI = true;
                    _CGI.push_back(CGI(res));
                    CGI &cgi = _CGI.back();
                    cgi.set_session_id(Singleton<SessionHandler>::get_instance()->get_session_id());
                    // char **envp = environ;
                    // while (*envp)
                    //     cgi.add_envp(*envp++);
                    cgi.add_envp("REQUEST_METHOD", req._method);
                    cgi.add_envp("SERVER_PROTOCOL", "HTTP/1.1");
                    cgi.add_envp("PATH_INFO", req._uri);
                    cgi.add_envp("QUERY_STRING", req._search);
                    cgi.add_envp("HTTP_SESSION", Singleton<SessionHandler>::get_instance()->get_session());
                    // std::cout << "------" << req._search << std::endl;

                    // cgi.add_envp("CONTENT_LENGTH", to_string(req._content_length));
                    // cgi.add_envp("CONTENT_TYPE", req._headers["Content-Type"]);
                    // cgi.add_envp("GATEWAY_INTERFACE", "CGI/1.1");
                    // cgi.add_envp("PATH_INFO", req._uri);
                    // cgi.add_envp("PATH_TRANSLATED", req._script_name);
                    // cgi.add_envp("REQUESTED_URI", req._uri);
                    // cgi.add_envp("REMOTE_ADDR", res._connection._client_ip);
                    // cgi.add_envp("SCRIPT_NAME", req._script_name);
                    // cgi.add_envp("SERVER_SOFTWARE", "webserv");
                    // cgi.add_envp("SERVER_PORT", to_string(res._connection._server_port));

                    for (std::map<std::string, std::string>::iterator it = req._headers.begin();
                            it != req._headers.end(); ++it)
                    {
                        if (it->first.find("X-") == 0 || it->first.find("Cookie") == 0)
                            cgi.add_envp("HTTP_" + it->first, it->second);
                    }
                    #if __APPLE__ && __MACH__
                    cgi.setup_bash("/usr/local/bin/python3", req._script_name, req._body);
                    #else
                    // cgi.setup_bash("ubuntu_cgi_tester", req._script_name, req._body);
                    cgi.setup_bash("/usr/bin/python3", req._script_name, req._body);
                    #endif
                    return;
                }
            }
            Middleware::execute(req, res);
        }
};


class Logger: public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            if (req._body.size())
            {
                std::ofstream ofs("body.log", std::ios::out | std::ios::trunc | std::ios::binary);
                ofs << req._body;   
            }
            Middleware::execute(req, res);
        }
};

class UploadDelete: public Middleware
{
    public:
        // todo think about http response status code for different failures
        void execute(Request &req, Response &res)
        {
            
            if (req._method == "POST" && checkMultiPart(req._headers["Content-Type"]) )
            {
                //get boundary information for headers
                std::string content_type = req._headers["Content-Type"];
                std::string::size_type pos = content_type.find("boundary=");
                if (pos != std::string::npos)
                {
                    std::string boundary = content_type.substr(pos + 9);
                    std::string body = req._body;
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
                    std::string filename = body.substr(pos_fn_start, pos_fn_end);
                    pos_fn_end = filename.find_last_of("\"");
                    filename = filename.substr(0, pos_fn_end);
                    // find emptyline
                    std::string::size_type pos_emptyline = body.find("\r\n\r\n");
                    body = body.substr(pos_emptyline + 4);
                    if (filename.empty())
                        throw HttpException(400, "Bad Request: filename not found");
                    // save file
                    std::ofstream ofs((req._script_name + filename).c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
                    ofs << body;
                    res.send_content(200, "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Upload Success</title></head><body> \
                    <h1>Upload Successful</h1><p>Your file has been successfully uploaded.</p> \
                    <p><a href=\"javascript:history.back()\">Go back</a></p></body></html>");
                }
                else
                    throw HttpException(400, "Bad Request: boundary not found");
            }
            // for curl & tester, not for browser multipart/form-data
            else if ((req._method == "POST" || req._method == "PUT"))
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
                std::string filename = req._script_name;
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
    private:
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
        bool checkMultiPart(std::string &content_type)
        {
            std::string::size_type pos = content_type.find("multipart/form-data");
            if (pos != std::string::npos)
                return true;
            return false;
        }
        void alert(std::stringstream &ss, const std::string &msg)
        {
            ss << "<script>alert(\"" << msg << "\");</script>";
        }
};

class KeepAliveHandler: public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            res.set_keep_alive(
                !req._headers.count("Connection")
                || req._headers["Connection"] != "close");
            Middleware::execute(req, res);
        }
};

class Pipeline : public Middleware
{
    public:
        Pipeline()
        {
            add(Singleton<ErrorPage>::get_instance());
            add(Singleton<SessionHandler>::get_instance());
            add(Singleton<Logger>::get_instance());
            add(Singleton<KeepAliveHandler>::get_instance());
            add(Singleton<CheckMethod>::get_instance());
            add(Singleton<Redirect>::get_instance());
            add(Singleton<CheckBodySize>::get_instance());
            add(Singleton<IndexFile>::get_instance()); // all methods
            add(Singleton<CgiRunner>::get_instance()); // all methods: upload.cgi?
            add(Singleton<UploadDelete>::get_instance()); // POST,PUT,DELETE must after cgi
            add(Singleton<DirectoryListing>::get_instance()); //GET
            add(Singleton<StaticFile>::get_instance()); //GET
        };

    private:
        void add(Middleware *middleware)
        {
            Middleware *walk = this;
            while (walk->getNext())
                walk = walk->getNext();
            walk->setNext(middleware);
        }
};

#endif
