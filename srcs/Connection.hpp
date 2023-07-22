#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "RequestBody.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

enum CONNECTION_STATUS {READING, SENDING, DISCONNECTED, WAITING};

class Connection
{
    public:
        Connection(int fd);
        Connection(const Connection &other);

        void    recv_request(void);
        void    set_response(const std::stringstream &data);
        void    send_response(void);
        bool    request_timeout(int sec);
        int     fd(void) const;
        void    set_keep_alive(bool keep_alive);
        bool    keep_alive(void) const;
        CONNECTION_STATUS &status(void);

        std::string     _request_buffer;
        std::ifstream   _ifile;
        int             _server_port;
        std::string     _server_ip;
        int             _client_port;
        std::string     _client_ip;
        int             _in_fd;

        RequestBody     request_body;

    private:
        Connection(void);
        size_t  next_connection_id(void);
        void    get_details(int connection_socket);
        void    on_send_complete(void);
        void    disconnect(void);
        void    transmit_file(void);

        std::string         _response_buffer;
        int                 _fd;
        CONNECTION_STATUS   _status;
        time_t              _last_activity;
        unsigned long       _start_time;
        bool                _keep_alive;
        const std::size_t   _id;
};

#endif
