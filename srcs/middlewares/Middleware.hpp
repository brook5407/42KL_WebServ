#ifndef MIDDLEWARE_HPP
#define MIDDLEWARE_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "HttpException.hpp"

#include <string>
#include <fstream>
#include <iostream>

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

#endif
