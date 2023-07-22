#ifndef HTTP_EXCEPTION_HPP
#define HTTP_EXCEPTION_HPP

#include <exception>
#include <string>

class HttpException: public std::exception
{
    public:
        HttpException(int status_code, const std::string &message);
        virtual ~HttpException() throw();
        virtual const char *what() const throw();
        int status_code() const;

    private:
        std::string _message;
        int _status_code;
};

#endif
