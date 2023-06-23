#include "Connection.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>


void Connection::read()
{
    _last_activity = time(NULL);
    if (_status != READING)
    {
        std::cout << "invalid status, expected READING. request-size:" << _in_buffer.size() << std::endl;
        // throw std::runtime_error("invalid status, expected READING");
        return;
    }
    char buffer[BUFFER_SIZE] = {};
    int length = recv(_fd, buffer, sizeof(buffer), 0); //MSG_NOSIGNAL
    if (length < 1)
    {
        _close();
        return;
    }
    _in_buffer += std::string(buffer, length);
    std::cout << "read " << length << " bytes from " << _fd <<  std::endl;
    // _status = _in_buffer.find("\r\n\r\n") != std::string::npos? READ: READING;
    //content-length & \r\n\r\n (handle malformed length), transfer-encoding: chunked
    //host mandatory
    //Connection: Keep-Alive | Keep-Alive: timeout=5, max=1000 | Connection: close
    // std::cout << _in_buffer  << " status " <<  _status << std::endl;
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
        int length = send(_fd, _out_buffer.c_str(), _out_buffer.size(), 0);
        if (length < 1)
        {
            _close();
            return;
        }
        std::cout << "SENT " << length << " to " << _fd <<  std::endl;
        _out_buffer = _out_buffer.substr(length);

        //stop sending when buffer sent and no file to send
        if (_out_buffer.empty() && !_ifile.is_open())
            _status = READING;
    }
    else
    {
        transmit_file();
    }
}

//todo test sending empty file
void Connection::transmit_file()
{
    if (_ifile.is_open())
    {
        if (!_ifile.eof())
        {
            char buffer[BUFFER_SIZE] = {};
            _ifile.read(buffer, sizeof(buffer));
            int sent = send(_fd, buffer, _ifile.gcount(), 0);
            if (sent < 1)
            {
                _ifile.close();
                _close();
                return;
            }
            if (sent < _ifile.gcount())
                _ifile.seekg(sent - _ifile.gcount(), std::ios::cur);
            std::cout << "SENT File " << sent << " to " << _fd <<  std::endl;
        }
        if (_ifile.eof()) //not else-if
        {
            _ifile.close();
            _status = READING; // reuse connection for next request
        }
    }
    else
    {
        _status = READING; //reuse connection for next request
    }
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
