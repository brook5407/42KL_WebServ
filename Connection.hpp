
enum CONNECTION_STATUS {READING, READ, SENDING, SENT, ERROR};

class Connection
{
    public:
        Connection() : _in_buffer(), _out_buffer(), _fd(0), _status(READING) {}
        Connection(int fd) : _in_buffer(), _out_buffer(), _fd(fd), _status(READING) {}
        ~Connection() {  } //close(_fd);
        int fd() const { return _fd; }
        //todo read body for cgi, upload
        void read()
        {
            if (_status != READING)
                throw std::runtime_error("invalid status");
            char buffer[1024] = {};
            int length = recv(_fd, buffer, sizeof(buffer) - 1, 0);
            std::cout << "read " << length << " bytes from " << _fd <<  std::endl;
            if (length < 0)
                throw std::runtime_error(strerror(errno));
            if (length)
                _in_buffer += std::string(buffer, length);
            _status = length == 0 || _in_buffer.find("\r\n\r\n") != std::string::npos? READ: READING;
            //content-length (handle malformed length)
            std::cout << _in_buffer  << " status " <<  _status << std::endl;
        }
        void write(const std::string &data)
        {
            std::cout << "write " << data << " to " << _fd <<  std::endl;
            if (_status != READ)
                throw std::runtime_error("invalid status");
            _out_buffer += data;
            _status = SENDING;
        }
        void transmit()
        {
            if (_status != READ && _status != SENDING)
                throw std::runtime_error("invalid status");
            int length = send(_fd, _out_buffer.c_str(), _out_buffer.size(), 0);
            if (length < 0)
                throw std::runtime_error(strerror(errno));
            if (length)
                _out_buffer = _out_buffer.substr(length);
            _status = _out_buffer.size() ? SENDING : SENT;
            if (_status == SENT)
                close(_fd);
        }
        void except()
        {
            std::cout << "except" << std::endl;
        }
        enum CONNECTION_STATUS status() const { return _status; }
        std::string _in_buffer;
        std::string _out_buffer;
    private:
        int _fd;
        enum CONNECTION_STATUS _status;
};
