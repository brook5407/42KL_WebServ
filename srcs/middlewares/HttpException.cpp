#include "HttpException.hpp"

HttpException::HttpException(int status_code, const std::string &message)
: _message(message), _status_code(status_code)
{}

HttpException::~HttpException() throw()
{}

const char *HttpException::what() const throw()
{
    return _message.c_str();
}

int HttpException::status_code() const
{
    return _status_code;
}
