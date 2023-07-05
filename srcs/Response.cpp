#include "Response.hpp"
#include "Singleton.hpp"
#include "SessionHandler.hpp"

void Response::send_location(int status_code, const std::string &location)
{
    std::stringstream ss;
    add_header(ss, status_code);
    ss << "Location: " << location << "\r\n\r\n";
    end(ss);
}

void Response::send_content(int status_code, const std::string &data, const std::string &type)
{
    // const int status_code = status == 0? 200 : _status;
    std::stringstream ss;
    add_header(ss, status_code);
    ss << "Content-Length: " << data.size() << "\r\n";
    if (!type.empty())
        ss << "Content-Type: " << type << "\r\n";
    // ss << "Connection: close\r\n";
    ss << "\r\n" << data;
    end(ss);
}

void Response::send_file(int status_code, const std::string &filepath)
{
    std::stringstream ss;
    add_header(ss, status_code);
    // _ifile triggers sendfile, todo clearer approach
    _connection._ifile.open(filepath.c_str(), std::ios::in | std::ios::binary);
    if (!_connection._ifile.is_open())
    {
        std::cout << "sendfile failed. no body is sent" << std::endl;
        ss << "Content-Length: 0\r\n\r\n";
        end(ss);
        return;
    }

    // _connection._ifile.seekg(0, std::ios::end);
    // std::size_t length = _connection._ifile.tellg();
    // _connection._ifile.seekg(0, std::ios::beg);
    // ss << "Content-Length: " << length << "\r\n";

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
        std::string content_type = "application/octet-stream";
        std::size_t pos = filepath.find_last_of(".");
        if (pos != std::string::npos)
        {
            std::string extension = filepath.substr(pos + 1);
            if (_configuration._mime_types.count(extension))
                content_type = _configuration._mime_types[extension];
        }
        ss << "Content-Type: " << content_type << "\r\n";
    }
    ss << "\r\n";
    end(ss);
}

void Response::send_cgi_fd(int fd, const std::string &session_id)
{
    char header[8192];
    off_t fsize = lseek(fd, 0, SEEK_END);
    //todo check fsize -1
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
    add_header(ss, 200); // todo status from stdout
    size_t cont_length = fsize - header_size;
    ss << "Content-Length: " << cont_length << "\r\n";
    std::stringstream cgi_ss(std::string(header, read_size));
    std::string cgi_line;
    // char session_key[] = "X-Replace-Session:";
    while (std::getline(cgi_ss, cgi_line))
    {
        if (cgi_line.empty() || cgi_line[0] == '\r')
            break;
        if (Singleton<SessionHandler>::get_instance()->parse_session(session_id, cgi_line) == true)
            continue;
        else
            ss << cgi_line << "\n";
    }
    ss << "\r\n";
    lseek(fd, header_size, SEEK_SET);
    _connection._in_fd = fd;
    end(ss);
    // std::cout << "send fd " << _fd << " sz:" << fsize << std::endl;
}

void Response::set_keep_alive(bool keep_alive)
{
    _connection.set_keep_alive(keep_alive);
}

void Response::add_header(std::stringstream &ss, int status_code)
{
    ss << "HTTP/1.1 " << status_code << ' ' << _configuration._reason_phrase[status_code] << "\r\n";
    if (_connection.keep_alive())
        ss << "Connection: keep-alive\r\n";
    else
        ss << "Connection: close\r\n";
    for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
        ss << it->first << ": " << it->second << "\r\n";
}

void Response::end(std::stringstream &ss)
{
    _connection.write(ss.str());
    _connection._in_buffer.clear();
}
