class Response
{
    public:
        Response(Connection &connection) : _connection(connection), _status(200), _is_ended(false) {}
        void write(const std::string &data)
        {
            _content += data;
        }
        void write_from_file(const std::string &filepath)
        {
            _filepath = filepath;
        }
        void status(int code)
        {
            _status = code;
        }
        int status()
        {
            return _status;
        }
        void header(const std::string &, const std::string &)
        {
            // _headers[key] = value;
        }
        void end(void)
        {
            _is_ended = true;
            std::stringstream ss;
            ss << "HTTP/1.1 " << _status << " OK\r\n";

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
                _connection._ifile.open(_filepath.c_str(), std::ios::in | std::ios::binary);
                if (!_connection._ifile.is_open())
                {
                    _connection.write("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
                    return;
                }
                _connection._ifile.seekg(0, std::ios::end);
                std::size_t length = _connection._ifile.tellg();
                _connection._ifile.seekg(0, std::ios::beg);
                // ss << "Content-Type: image/jpeg\r\n";
                ss << "Content-Length: " << length << "\r\n";
                // ss << "Connection: close\r\n";
                ss << "\r\n";
                // ss << _connection._ifile.rdbuf();
            }
            _connection.write(ss.str());
        }
        bool is_ended(void)
        {
            return _is_ended;
        }

    private:
        Connection &_connection;
        int _status;
        std::string _content;
        std::string _filepath;
        bool _is_ended;
};