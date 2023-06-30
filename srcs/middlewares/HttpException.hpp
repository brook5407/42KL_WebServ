#ifndef HTTP_EXCEPTION_HPP
#define HTTP_EXCEPTION_HPP

#include <exception>

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

#endif
