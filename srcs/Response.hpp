#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Connection.hpp"
#include <map>

class Response
{
    public:
        Response(Connection &connection);
        Response(Response const &src);

        void send_location(int status_code, const std::string &location);
        void send_content(int status_code, const std::string &data, const std::string &type = "text/html");
        void send_file(int status_code, const std::string &filepath, const std::string &mimetype);
        void send_error_file(int status_code, const std::string &filepath);
        void send_cgi_fd(int fd, const std::string &session_id);
        void set_keep_alive(bool keep_alive);
        void set_header(const std::string &key, const std::string &value)
        {
            _headers[key] = value;
        }

        Connection &_connection;

    private:
        std::map<std::string, std::string> _headers;
        Response(void);
        void add_response_header(std::stringstream &ss, int status_code);
};

#endif
