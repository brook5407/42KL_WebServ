#ifndef MIDDLEWARE_HPP
#define MIDDLEWARE_HPP

#include "Request.hpp"
#include "Response.hpp"

#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include "CGI.hpp"

#ifndef DT_DIR
# define DT_DIR  4
# define DT_REG  8
#endif

extern char **environ;

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os;
    os << value;
    return os.str();
}

template <typename T>
struct Singleton
{
    static T *get_instance()
    {
        static T instance;
        return &instance;
    }
};

class HttpException: public std::exception
{
    public:
        HttpException(int status_code, const std::string &message)
            : _message(message), _status_code(status_code) {}
        virtual ~HttpException() throw() {}
        virtual const char *what() const throw()
        {
            return _message.c_str();
        }
        int status_code() const throw()
        {
            return _status_code;
        }
    private:
        std::string _message;
        int _status_code;
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

        // add check file function here
        bool	file_exists(std::string &route)
        {
	        std::ifstream	file(route.c_str());
	        return file.good();
        }

        bool   file_executable(std::string &route)
        {
            return (access(route.c_str(), X_OK) == 0);
        }
        bool    file_extension(std::string &route, const std::string extension)
        {
            return (route.substr(route.find_last_of('.')) == extension);
        }

    protected:
        Middleware *_next;
};

class CheckMethod : public Middleware
{
    public:
        // todo: get allowed method by route
        void execute(Request &req, Response &res)
        {
            if (req._method != "GET" && req._method != "POST"
                    && req._method != "DELETE" && req._method != "PUT")
                throw HttpException(405, "Method Not Allowed");
            Middleware::execute(req, res);
        }
};

class IndexFile : public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            if ((*req._route)["index"].size())
            {
                DIR *dir = opendir(req._script_name.c_str());
                if (dir)
                {
                    closedir(dir);

                    std::string index_file = req._script_name;
                    if (req._script_name[req._script_name.size() - 1] != '/')
                        index_file += "/";
                    index_file += (*req._route)["index"];
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

            dir = opendir(req._script_name.c_str());
            if (dir == NULL)
                return Middleware::execute(req, res);

            res.write("<html><body>");
            res.write("<h1>Directory listing</h1>");
            res.write("<ol>");

            while ((entry = readdir(dir)) != NULL)
            {
                // hide dot files
                if (entry->d_name[0] == '.')
                    continue;
                if (entry->d_type != DT_DIR && entry->d_type != DT_REG)
                    continue;
                res.write(std::string() + "<li><a href=\"" + req._uri);
                // add directory slash if not present
                if (req._uri[req._uri.size() - 1] != '/')
                    res.write("/");
                res.write(std::string() + entry->d_name);
                // append slash to directory
                if (entry->d_type == DT_DIR)
                    res.write("/");
                res.write(std::string() + "\">" + entry->d_name);
                if (entry->d_type == DT_DIR)
                    res.write("/");
                res.write("</a></li>");
            }

            closedir(dir);
            res.write("</ol>");
            res.write("</html></body>");
        }
};

class Redirect: public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            if (false)
            {
                res.status(301);
                res.header("Location", "http://www.google.com");
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
                res.send_file(req._script_name);
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
                res.send_file(error_page);
            }
        }
};

class CgiRunner: public Middleware
{
    public:
        std::list<CGI>  _CGI;
        bool    is_CGI;
        void execute(Request &req, Response &res)
        {
            if (req._route->find("cgi") != req._route->end())
            {
                is_CGI = false;
                std::string extensions[] = {(*req._route)["cgi"]};
                for (size_t i = 0; i < 1; ++i)
                {
                    const std::string &extension =  extensions[i];
                    if (file_extension(req._script_name, extension))
                    {
                        if (!file_exists(req._script_name))
                            throw HttpException(404, "File not found");
                        if (!file_executable(req._script_name))
                            throw HttpException(403, "File is not executable");
                        is_CGI = true;
                        _CGI.push_back(CGI(res));
                        CGI &cgi = _CGI.back();
                        
                        // cgi.add_environ loop
                        char **envp = environ;
                        while (*envp)
                        {
                            cgi.add_envp(*envp);
                            envp++;
                        }
                        for (std::map<std::string, std::string>::iterator it = req._headers.begin(); it != req._headers.end(); ++it)
                        {
                            std::string input = (it->first + "=" + it->second).c_str();
                            cgi.add_envp(input.c_str());
                        }
                        // cgi.add_environ(QUERY_STRING=req._search);
                        cgi.add_CGI_var(req);
                        cgi.setup_bash(req._script_name);
                        return;
                    }
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
                save_file(filename, req._body);
                res.write(filename.substr(filename.find_last_of('/') + 1) + " has been uploaded!");
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
            if (content.empty())
                throw HttpException(400, "Bad Request: no content");
            std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
            if (ofs.fail())
                throw HttpException(500, "Internal Server Error: fail to open file");
            ofs << content;
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
            add(Singleton<CheckMethod>::get_instance());
            add(Singleton<Redirect>::get_instance());
            add(Singleton<IndexFile>::get_instance()); // all methods
            add(Singleton<CgiRunner>::get_instance()); // all methods: upload.cgi?
            add(Singleton<UploadDelete>::get_instance()); // POST,PUT,DELETE must after cgi
            add(Singleton<DirectoryListing>::get_instance()); //GET
            add(Singleton<StaticFile>::get_instance()); //GET
        };
        void add(Middleware *middleware)
        {
            Middleware *walk = this;
            while (walk->getNext())
                walk = walk->getNext();
            walk->setNext(middleware);
        }
};

#endif
