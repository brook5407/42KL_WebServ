#include "Response.hpp"
#include "Singleton.hpp"
#include "SessionHandler.hpp"
#include "ReasonPhrase.hpp"
#include "Util.hpp"
#include <sys/stat.h>
#include <ctime>
#include <algorithm>

Response::Response(Connection &connection)
: _connection(connection)
{
    _connection.status() = WAITING;
}

Response::Response(Response const &src)
: _connection(src._connection), _headers(src._headers)
{}

void Response::send_location(int status_code, const std::string &location)
{
    std::stringstream ss;
    add_response_header(ss, status_code);
    ss << "Location: " << location << "\r\n"
        "Content-Length: 0\r\n"
        "\r\n";
    _connection.set_response(ss);
}

void Response::send_content(int status_code, const std::string &data, const std::string &type)
{
    std::stringstream ss;
    add_response_header(ss, status_code);
    ss << "Content-Length: " << data.size() << "\r\n";
    if (!type.empty())
        ss << "Content-Type: " << type << "\r\n";
    ss << "\r\n" << data;
    _connection.set_response(ss);
}

void Response::send_file(int status_code, const std::string &filepath, const std::string &mimetype)
{
    std::stringstream ss;
    add_response_header(ss, status_code);
    _connection._ifile.open(filepath.c_str(), std::ios::in | std::ios::binary);
    if (!_connection._ifile.is_open())
    {
        std::cout << "sendfile failed. no body is sent" << std::endl;
        ss << "Content-Length: 0\r\n\r\n";
        _connection.set_response(ss);
        return;
    }

    struct stat fileStat;
    if (stat(filepath.c_str(), &fileStat) == 0)
    {
        time_t modifiedTime = fileStat.st_mtime;
        char timeBuffer[100];
        strftime(timeBuffer, sizeof(timeBuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&modifiedTime));
        ss << "Content-Length: " << fileStat.st_size << "\r\n";
        ss << "Last-Modified: " << timeBuffer << "\r\n"; // for caching
    }

    {
        ss << "Content-Type: " << mimetype << "\r\n";
    }
    ss << "\r\n";
    _connection.set_response(ss);
}

void Response::send_error_file(int status_code, const std::string &filepath)
{
    send_file(status_code, filepath, "text/html");
}

void Response::send_cgi_fd(int fd, const std::string &session_id)
{
    char header[8192];
    off_t fsize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    int read_size = read(fd, header, sizeof(header));
    if (read_size < 0)
    {
        std::cout << "read failed" << std::endl;
        return;
    }
    size_t header_size = 0;
    char *it = std::search(header, header + read_size, "\r\n\r\n", (const char *)("\r\n\r\n") + 4);
    if (it < header + read_size)
    {
        header_size = it - header + 4;
    }
    else
    {
        it = std::search(header, header + read_size, "\n\n", (const char *)("\n\n") + 2);
        if (it < header + read_size)
            header_size = it - header + 2;
    }
    std::stringstream ss;
    add_response_header(ss, 200); // todo status from stdout
    size_t cont_length = fsize - header_size;
    ss << "Content-Length: " << cont_length << "\r\n";
    std::stringstream cgi_ss(std::string(header, read_size));
    std::string cgi_line;
    while (std::getline(cgi_ss, cgi_line))
    {
        if (cgi_line.empty() || cgi_line[0] == '\r')
            break;
        if (Singleton<SessionHandler>::get_instance().parse_session(session_id, cgi_line) == true)
            continue;
        else
            ss << cgi_line << "\n";
    }
    ss << "\r\n";
    lseek(fd, header_size, SEEK_SET);
    _connection._in_fd = fd;
    _connection.set_response(ss);
}

void Response::set_keep_alive(bool keep_alive)
{
    _connection.set_keep_alive(keep_alive);
}

void Response::add_response_header(std::stringstream &ss, int status_code)
{
    ss  << "HTTP/1.1 "
        << status_code << ' '
        << Singleton<ReasonPhrase>::get_instance().lookup(status_code)
        << "\r\n";
    if (_connection.keep_alive())
        ss << "Connection: keep-alive\r\n";
    else
        ss << "Connection: close\r\n";
    for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
        ss << it->first << ": " << it->second << "\r\n";
}
