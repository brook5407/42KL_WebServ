#include "Connection.hpp"
#include <iostream>

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

        //todo maybe check no file then end this earlier
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
        _close();
        std::cout << "timeout " << _fd << " after " << duration_sec << std::endl;
        return true;
    }
    return false;
}