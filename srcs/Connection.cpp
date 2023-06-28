#include "Connection.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>

    //Connection: Keep-Alive | Keep-Alive: timeout=5, max=1000 | Connection: close

static unsigned long get_nanosecond()
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

char _buffer[BUFFER_SIZE] = {};


void Connection::read()
{
    _last_activity = time(NULL);
    if (_status != READING)
    {
        std::cout << "invalid status, expected READING. request-size:" << _in_buffer.size() << std::endl;
        // throw std::runtime_error("invalid status, expected READING");
        return;
    }
    // char buffer[BUFFER_SIZE] = {};
    int length = recv(_fd, _buffer, sizeof(_buffer), 0); //MSG_NOSIGNAL
    if (length < 1)
    {
        _close();
        return;
    }
    if (_in_buffer.empty())
        _start_time = get_nanosecond();
    _in_buffer += std::string(_buffer, length);
    // std::cout << "read " << length << " bytes from " << _fd <<  std::endl;
}

void Connection::write(const std::string &data)
{
    if (_status != READING)
    {
        std::cout << "existing " << _out_buffer << std::endl << "===" << data << std::endl;
        throw std::runtime_error("invalid status, expected READING ");
    }
    _out_buffer += data;
    _status = SENDING;
}

void Connection::transmit()
{
    _last_activity = time(NULL);
    _status = SENDING;
    // if (_status != READ && _status != SENDING)
    //     throw std::runtime_error("invalid status, expected READ or SENDING");

    if (!_out_buffer.empty())
    {
        int length = send(_fd, _out_buffer.c_str(),
            (_out_buffer.size() > BUFFER_SIZE? BUFFER_SIZE: _out_buffer.size()), 0);
        if (length < 1)
        {
            _close();
            return;
        }
        // std::cout << "SENT " << length << " to " << _fd <<  std::endl;
        _out_buffer = _out_buffer.substr(length);

        //stop sending when buffer sent and no file to send
        if (_out_buffer.empty() && !_ifile.is_open() && _in_fd == -1)
        {
            _status = READING;
            show_duration();
        }
    }
    else
    {
        transmit_file();
    }
}

//todo test sending empty file
void Connection::transmit_file()
{
    // char buffer[BUFFER_SIZE] = {};
    if (_in_fd > -1)
    {
        int sz_read = ::read(_in_fd, _buffer, sizeof(_buffer));
        // std::cout << "sending " << sz_read << " fd:" << _in_fd << std::endl;
        if (sz_read < 0)
            perror("read cgi-stdout");
        if (sz_read < 1)
        {
            close(_in_fd);
            _in_fd = -1;
            show_duration();
            _status = READING; // reuse connection for next request
            return;
        }
        int sent = send(_fd, _buffer, sz_read, 0);
        if (sent < 1)
        {
            close(_in_fd);
            _in_fd = -1;
            _close();
            return;
        }
        if (sent < sz_read)
            lseek(_in_fd, sent - sz_read, SEEK_CUR);
        // std::cout << "SENT STDOUT " << sent << " to " << _fd <<  std::endl;
    }
    else 
    if (_ifile.is_open())
    {
        if (!_ifile.eof())
        {
            // char buffer[BUFFER_SIZE] = {};
            _ifile.read(_buffer, sizeof(_buffer));
            int sent = send(_fd, _buffer, _ifile.gcount(), 0);
            if (sent < 1)
            {
                _ifile.close();
                _close();
                return;
            }
            if (sent < _ifile.gcount())
                _ifile.seekg(sent - _ifile.gcount(), std::ios::cur);
            // std::cout << "SENT File " << sent << " to " << _fd <<  std::endl;
        }
        if (_ifile.eof()) //not else-if
        {
            _ifile.close();
            show_duration();
            _status = READING; // reuse connection for next request
        }
    }
    else
    {
        show_duration();
        _status = READING; //reuse connection for next request
    }
}

void Connection::show_duration()
{
    static size_t total = 0;
    std::cout << "Request #" << ++total << " duration: " << format_nanosecond(get_nanosecond() - _start_time) << std::endl;
}

void Connection::except()
{
    std::cout << "except" << std::endl;
}

void Connection::_close()
{
    if (_fd > 0)
    {
        close(_fd);
        _fd = -1;
        _status = CLOSED;
    }
}
// different timeout for idle, long read, long write?
bool Connection::is_timeout(int sec)
{
    const double duration_sec = difftime(time(NULL), _last_activity);
    if (duration_sec > sec)
    {
        std::cout << "timeout #" << _fd << " after " << duration_sec << std::endl;
        _close();
        return true;
    }
    return false;
}

void Connection::get_details(int connection_socket) {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    if (getpeername(connection_socket, (struct sockaddr*)&client_address, &client_address_len) == -1)
        // throw std::runtime_error(strerror(errno));
        perror("Connection");

    struct sockaddr_in server_address;
    socklen_t server_address_len = sizeof(server_address);
    if (getsockname(connection_socket, (struct sockaddr*)&server_address, &server_address_len) == -1)
        // throw std::runtime_error(strerror(errno));
        perror("Connection");

    char client_ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL)
        // throw std::runtime_error(strerror(errno));
        perror("Connection");

    char server_ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(server_address.sin_addr), server_ip, INET_ADDRSTRLEN) == NULL)
        // throw std::runtime_error(strerror(errno));
        perror("Connection");

    _client_ip = std::string(client_ip);
    _server_ip = std::string(server_ip);
    _client_port = ntohs(client_address.sin_port);
    _server_port = ntohs(server_address.sin_port);
    
}

std::ostream& operator<<(std::ostream& os, const Connection& connection)
{
    os << "Connection #" << connection._fd << " "
     << connection._client_ip << ":" << connection._client_port 
     << (connection._status == READING? " > ": connection._status == SENDING? " < ": " - ")
     << connection._server_ip << ":" << connection._server_port;
    return os;
}
