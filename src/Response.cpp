#include "Response.hpp"

void Response::end(void)
{
    const int status_code = _status == 0? 200 : _status;
    std::stringstream ss;
    ss << "HTTP/1.1 " << status_code << ' ' << _configuration._reason_phrase[status_code] << "\r\n";

    if (_filepath.empty())
    {
        ss << "Content-Length: " << _content.size() << "\r\n";
        ss << "Content-Type: text/html\r\n";
        // ss << "Connection: close\r\n";
        ss << "\r\n";
        ss << _content;
    }
    else
    {
        // _ifile triggers sendfile, todo clearer approach
        _connection._ifile.open(_filepath.c_str(), std::ios::in | std::ios::binary);
        if (!_connection._ifile.is_open())
        {
            std::cout << "sendfile failed. no body is sent" << std::endl;
            ss << "Content-Length: 0\r\n\r\n";
            _connection.write(ss.str());
            // _connection.write("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
            _connection._in_buffer.clear(); // todo: refactor?
            return;
        }

        // _connection._ifile.seekg(0, std::ios::end);
        // std::size_t length = _connection._ifile.tellg();
        // _connection._ifile.seekg(0, std::ios::beg);
        // ss << "Content-Length: " << length << "\r\n";

        struct stat fileStat;
        if (stat(_filepath.c_str(), &fileStat) == 0)
        {
            time_t modifiedTime = fileStat.st_mtime;
            char timeBuffer[100];
            strftime(timeBuffer, sizeof(timeBuffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&modifiedTime));
            ss << "Content-Length: " << fileStat.st_size << "\r\n";
            // todo enable below after testing
            // ss << "Last-Modified: " << timeBuffer << "\r\n";
        }

        {
            std::string content_type = "application/octet-stream";
            std::size_t pos = _filepath.find_last_of(".");
            if (pos != std::string::npos)
            {
                std::string extension = _filepath.substr(pos + 1);
                if (_configuration._mime_types.count(extension))
                    content_type = _configuration._mime_types[extension];
            }
            ss << "Content-Type: " << content_type << "\r\n";
        }

        // todo add headers
        // ss << "Connection: close\r\n";
        ss << "\r\n";
        // ss << _connection._ifile.rdbuf();
    }
    _connection.write(ss.str());
    _connection._in_buffer.clear(); //!!!! this is important for all exit branch !!!
}
