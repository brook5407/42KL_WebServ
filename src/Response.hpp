#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Configuration.hpp"
#include "Connection.hpp"
#include <sys/stat.h>
#include <ctime>

class Response
{
    public:
        Response(Connection &connection, Configuration &configuration)
            : _connection(connection), _configuration(configuration),  _status(0) {}

        Response(Response const &src)
            : _connection(src._connection), _configuration(src._configuration), 
             _status(src._status),_content(src._content), _filepath(src._filepath) {}

        void write(const std::string &data)
        {
            _content += data;
        }
        void send_file(const std::string &filepath)
        {
            _filepath = filepath;
        }
        void status(int code)
        {
            _status = code;
        }
        // int status()
        // {
        //     return _status;
        // }
        void header(const std::string &, const std::string &)
        {
            // _headers[key] = value;
        }
        void end(void);

    private:
        Connection &_connection;
        Configuration &_configuration;
        int _status;
        std::string _content;
        std::string _filepath;
};

#endif
