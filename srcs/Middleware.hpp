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
                    // char **envp = environ;
                    // while (*envp)
                    //     cgi.add_envp(*envp++);
                    cgi.add_envp("REQUEST_METHOD", req._method);
                    cgi.add_envp("SERVER_PROTOCOL", "HTTP/1.1");
                    cgi.add_envp("PATH_INFO", req._uri);
                    cgi.add_envp("QUERY_STRING", req._search);

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
                        if (it->first.find("X-") == 0)
                            cgi.add_envp("HTTP_" + it->first, it->second);
                    }
                    #if __APPLE__ && __MACH__
                    cgi.setup_bash("cgi_tester", req._script_name, req._body);
                    #else
                    cgi.setup_bash("ubuntu_cgi_tester", req._script_name, req._body);
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
            add(Singleton<Logger>::get_instance());
            // add(Singleton<Session>::get_instance());
            add(Singleton<KeepAliveHandler>::get_instance());
            add(Singleton<CheckMethod>::get_instance());
            add(Singleton<Redirect>::get_instance());
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
