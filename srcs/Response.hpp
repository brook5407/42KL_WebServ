#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Configuration.hpp"
#include "Connection.hpp"
#include <sys/stat.h>
#include <ctime>
#include <algorithm>

class Response
{
    public:
        Response(Connection &connection, Configuration &configuration)
            : _connection(connection), _configuration(configuration) {}

        Response(Response const &src)
            : _connection(src._connection), _configuration(src._configuration) 
            //  _headers(src._headers) 
            {}

        void send_location(int status_code, const std::string &location);
        void send_content(int status_code, const std::string &data, const std::string &type = "text/html");
        void send_file(int status_code, const std::string &filepath);
        void send_cgi_fd(int fd, const std::string &session_id);
        void set_keep_alive(bool keep_alive);
        void set_header(const std::string &key, const std::string &value)
        {
            _headers[key] = value;
        }

        Connection &_connection;
        std::map<std::string, std::string> _headers;
    private:
        Configuration &_configuration;

        void add_header(std::stringstream &ss, int status_code);
        void end(std::stringstream &ss);
};

#endif
