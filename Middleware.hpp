#ifndef MIDDLEWARE_HPP
#define MIDDLEWARE_HPP

#include "Request.hpp"
#include "Response.hpp"

#include <string>
#include <dirent.h>

#define DT_DIR  4
#define DT_REG  8

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
        HttpException(int status_code, const std::string &message): _message(message), _status_code(status_code) {}
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
                res.write_from_file(req._script_name);
            else
                throw HttpException(404, "Not found");
        }
};

// todo this should be part of Response, instead of middleware?
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
                res.write_from_file(error_page);
            }
        }
};

class CgiRunner: public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            if (req._route->find("cgi") != req._route->end())
            {
                const std::string extension = (*req._route)["cgi"];
                if (req._script_name.size() > extension.size())
                {
                    if (req._script_name.find(extension, req._script_name.size() - extension.size())
                            != std::string::npos)
                    {
                        throw HttpException(500, "Internal Server Error");
                        // fork and exec cgi
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
            add(Singleton<IndexFile>::get_instance());
            add(Singleton<CgiRunner>::get_instance()); // upload.cgi?
            add(Singleton<DirectoryListing>::get_instance());
            add(Singleton<StaticFile>::get_instance());
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
