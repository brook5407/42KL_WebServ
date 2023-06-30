#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#define BUFFER_SIZE 100 * 1024 * 1024
enum CONNECTION_STATUS {READING, SENDING, CLOSED};

class Connection
{
    public:
        Connection() :
            _in_buffer(),  _ifile(), _server_port(), _in_fd(-1), _out_buffer(), _fd(0),
            _status(READING), _last_activity(time(NULL)), _keep_alive(true)
            {}
        Connection(const Connection &other)
            : _in_buffer(),  _ifile(), 
            _server_port(other._server_port), 
            _server_ip(other._server_ip),
            _client_port(other._client_port),
            _client_ip(other._client_ip),
            _in_fd(other._in_fd),
            _out_buffer(),_fd(other._fd), 
            _status(other._status), _last_activity(other._last_activity),
            _keep_alive(other._keep_alive)
            {

            } 
        Connection(int fd)
            : _in_buffer(), _ifile(), _in_fd(-1), _out_buffer(), _fd(fd), _status(READING),
            _last_activity(time(NULL)), _keep_alive(true)
            {
                get_details(fd);
            }
        ~Connection() { } //close(_fd); // avoid auto-close due to copy
        Connection &operator=(const Connection &)
        {
            throw std::runtime_error("Connection assignment operator not implemented");
        }

        int fd() const { return _fd; }
        bool is_timeout(int sec);
        void except();
        void transmit();
        void write(const std::string &data);
        void read();
        enum CONNECTION_STATUS status() const { return _status; }
    	friend std::ostream& operator<<(std::ostream& os, const Connection& connection);
        void set_keep_alive(bool keep_alive) { _keep_alive = keep_alive; }
        bool keep_alive() const { return _keep_alive; }

        std::string _in_buffer;
        std::ifstream _ifile;
        int _server_port;
        std::string _server_ip;
        int _client_port;
        std::string _client_ip;
        int _in_fd;
    private:
        void get_details(int connection_socket);
        void on_send_complete(void);
        void _close();
        void transmit_file();

        std::string _out_buffer;
        int _fd;
        enum CONNECTION_STATUS _status;
        time_t _last_activity;
        unsigned long _start_time;
        bool _keep_alive;
};

#endif
