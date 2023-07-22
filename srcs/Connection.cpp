#include "Connection.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <cstdlib>
#include <algorithm>

static char _buffer[BUFFER_SIZE];

static unsigned long get_nanosecond(void)
{
    struct timespec currentTime;

    clock_gettime(CLOCK_MONOTONIC_RAW, &currentTime);
    return static_cast<unsigned long>(currentTime.tv_sec) * 1000000000L + currentTime.tv_nsec;
}

static std::string format_nanosecond(double nanoseconds)
{
    std::stringstream ss;
    if (nanoseconds < 1000)
        ss << nanoseconds << " ns";
    else if (nanoseconds < 1000000)
        ss << nanoseconds / 1000. << " µs";
    else if (nanoseconds < 1000000000)
        ss << nanoseconds / 1000000. << " ms";
    else
        ss << nanoseconds / 1000000000. << " s";
    return ss.str();
}

std::size_t Connection::next_connection_id(void)
{
    static std::size_t id = 0;
    return ++id;
}

Connection::Connection(int fd)
: _request_buffer(), _ifile(), _in_fd(-1), _response_buffer(), _fd(fd), _status(READING),
_last_activity(time(NULL)), _keep_alive(true), _id(next_connection_id())
{
    get_details(fd);
}

Connection::Connection(const Connection &other)
: _request_buffer(),  _ifile(),
_server_port(other._server_port),
_server_ip(other._server_ip),
_client_port(other._client_port),
_client_ip(other._client_ip),
_in_fd(other._in_fd),
_response_buffer(),_fd(other._fd),
_status(other._status), _last_activity(other._last_activity),
_keep_alive(other._keep_alive),
_id(other._id)
{
}

int Connection::fd(void) const { return _fd; }
CONNECTION_STATUS  &Connection::status(void) { return _status; }
void    Connection::set_keep_alive(bool keep_alive) { _keep_alive = keep_alive; }
bool    Connection::keep_alive(void) const { return _keep_alive; }

void Connection::recv_request(void)
{
    _last_activity = time(NULL);
    int length = recv(_fd, _buffer, sizeof(_buffer), 0); //MSG_NOSIGNAL
    if (length < 1)
    {
        disconnect();
        return;
    }
    if (_request_buffer.empty())
        _start_time = get_nanosecond();
    if (_request_buffer.find("\r\n\r\n") == std::string::npos)
    {
        char *buffer_end = _buffer + length;
        char *blank_line = std::search(_buffer, buffer_end, "\r\n\r\n", "\r\n\r\n" + 4);
        _request_buffer.append(_buffer, std::min(blank_line + 4, buffer_end));

std::cout << _request_buffer.size() << std::endl;
        // std::cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        // << std::endl << _request_buffer << std::endl
        // << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << std::endl;
        if (blank_line < buffer_end)
        {
            if (_request_buffer.find(": chunked\r\n") != std::string::npos)
                request_body.init_chunk();
            else
            {
                size_t content_length = 0;
                const char cl[] = "Content-Length:";
                size_t pos_cl = _request_buffer.find(cl);
                if (pos_cl != std::string::npos)
                    content_length = std::atoi(_request_buffer.c_str() + pos_cl + sizeof(cl));
                request_body.init_nonchunk(content_length);
            }
            request_body.add_chunk(blank_line + 4, buffer_end - blank_line - 4);
        }
    }
    else
        request_body.add_chunk(_buffer, length);
}

void Connection::set_response(const std::stringstream &data)
{
    _response_buffer = data.str();
    _status = SENDING;
    _request_buffer.clear();
}

void Connection::send_response(void)
{
    _last_activity = time(NULL);
    if (!_response_buffer.empty())
    {
        int length = send(_fd, _response_buffer.c_str(), std::min((size_t)BUFFER_SIZE, _response_buffer.size()), 0);
        if (length < 1)
        {
            disconnect();
            return;
        }
        _response_buffer.erase(0, length);

        //stop sending when buffer sent and no file to send
        if (_response_buffer.empty() && !_ifile.is_open() && _in_fd == -1)
            on_send_complete();
    }
    else
    {
        transmit_file();
    }
}

void Connection::transmit_file()
{
    if (_in_fd > -1)
    {
        int sz_read = read(_in_fd, _buffer, sizeof(_buffer));
        if (sz_read < 0)
            perror("read cgi-stdout");
        if (sz_read < 1)
        {
            close(_in_fd);
            _in_fd = -1;
            on_send_complete();
            return;
        }
        int sent = send(_fd, _buffer, sz_read, 0);
        if (sent < 1)
        {
            close(_in_fd);
            _in_fd = -1;
            disconnect();
            return;
        }
        if (sent < sz_read)
            lseek(_in_fd, sent - sz_read, SEEK_CUR);
    }
    else if (_ifile.is_open())
    {
        if (!_ifile.eof())
        {
            _ifile.read(_buffer, sizeof(_buffer));
            int sent = send(_fd, _buffer, _ifile.gcount(), 0);
            if (sent < 1)
            {
                _ifile.close();
                disconnect();
                return;
            }
            if (sent < _ifile.gcount())
                _ifile.seekg(sent - _ifile.gcount(), std::ios::cur);
        }
        if (_ifile.eof()) //not else-if
        {
            _ifile.close();
            on_send_complete();
        }
    }
    else
    {
        on_send_complete();
    }
}

void Connection::on_send_complete()
{
    static size_t total = 0;
    std::cout
        << "Request " << ++total
        << " Connection #" << _id
        << " duration " << format_nanosecond(get_nanosecond() - _start_time)
        << std::endl;
    if (_keep_alive)
        _status = READING;
    else
        disconnect();
}

void Connection::disconnect(void)
{
    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
    _status = DISCONNECTED;
}

// different timeout for idle, long read, long write?
bool Connection::request_timeout(int sec)
{
    if (_status != READING || !_request_buffer.empty())
        return false;
    const double duration_sec = difftime(time(NULL), _last_activity);
    if (duration_sec >= sec)
    {
        std::cout << "Connection #" << _id << " timeout after " << duration_sec << " s" << std::endl;
        disconnect();
        return true;
    }
    return false;
}

void Connection::get_details(int connection_socket)
{
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    if (getpeername(connection_socket, (struct sockaddr*)&client_address, &client_address_len) == -1)
        perror("getpeername");

    struct sockaddr_in server_address;
    socklen_t server_address_len = sizeof(server_address);
    if (getsockname(connection_socket, (struct sockaddr*)&server_address, &server_address_len) == -1)
        perror("getsockname");

    char client_ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL)
        perror("inet_ntop/1");

    char server_ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(server_address.sin_addr), server_ip, INET_ADDRSTRLEN) == NULL)
        perror("inet_ntop/2");

    _client_ip = std::string(client_ip);
    _server_ip = std::string(server_ip);
    _client_port = ntohs(client_address.sin_port);
    _server_port = ntohs(server_address.sin_port);
}
