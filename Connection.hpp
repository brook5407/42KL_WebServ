#include <fstream>
#define BUFFER_SIZE 65536
enum CONNECTION_STATUS {READING, READ, SENDING, SENT, ERROR};

class Connection
{
    public:
        Connection() : _in_buffer(), _out_buffer(), _ifile(), _fd(0), _status(READING) {}
        Connection(const Connection &other) : _in_buffer(), _out_buffer(), _ifile(), _fd(other._fd), _status(other._status) { } 
        Connection(int fd) : _in_buffer(), _out_buffer(), _ifile(), _fd(fd), _status(READING) {}
        ~Connection() {  } //close(_fd);
        Connection &operator=(const Connection &other)
        {
            _fd = other._fd;
            // _in_buffer = other._in_buffer;
            // _out_buffer = other._out_buffer;
            // _status = other._status;
            return *this;
        }
        int fd() const { return _fd; }
        void read()
        {
            if (_status != READING)
                throw std::runtime_error("invalid status");
            char buffer[BUFFER_SIZE] = {};
            int length = recv(_fd, buffer, sizeof(buffer) - 1, 0); //MSG_NOSIGNAL
            std::cout << "read " << length << " bytes from " << _fd <<  std::endl;
            if (length < 0)
                throw std::runtime_error(strerror(errno));
            if (length)
                _in_buffer += std::string(buffer, length);
            _status = length == 0 || _in_buffer.find("\r\n\r\n") != std::string::npos? READ: READING;
            //content-length & \r\n\r\n (handle malformed length), transfer-encoding: chunked
            //host mandatory, connection: close,
            //keep-alive
            std::cout << _in_buffer  << " status " <<  _status << std::endl;
        }
        void write(const std::string &data)
        {
            if (_status != READ)
                throw std::runtime_error("invalid status");
            _out_buffer += data;
            _status = SENDING;
        }
        void transmit()
        {
            _status = SENDING;
            if (_status != READ && _status != SENDING)
                throw std::runtime_error("invalid status");
            int length = send(_fd, _out_buffer.c_str(), _out_buffer.size(), 0);
            if (length < 0)
                throw std::runtime_error(strerror(errno));
            if (length)
            {
                std::cout << "SENT " << length << " to " << _fd <<  std::endl;
                _out_buffer = _out_buffer.substr(length);
            }
            else if (_out_buffer.empty())
                transmit_file();
        }
        void transmit_file()
        {
            if (_ifile.is_open() && !_ifile.eof())
            {
                char buffer[BUFFER_SIZE] = {};
                _ifile.read(buffer, sizeof(buffer));
                int sent = send(_fd, buffer, _ifile.gcount(), 0);
                if (sent < 0)
                    throw std::runtime_error(strerror(errno));
                if (sent < _ifile.gcount())
                    _ifile.seekg(sent - _ifile.gcount(), std::ios::cur);
                std::cout << "SENT File " << sent << " to " << _fd <<  std::endl;
            }
            else
            {
                _status = SENT;
                close(_fd);
            }
        }
        void except()
        {
            std::cout << "except" << std::endl;
        }
        enum CONNECTION_STATUS status() const { return _status; }
        std::string _in_buffer;
        std::string _out_buffer;
        std::ifstream _ifile;
    private:
        int _fd;
        enum CONNECTION_STATUS _status;
};
