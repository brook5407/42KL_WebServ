
class Response
{
    public:
        Response(Connection &connection) : _connection(connection) {}
        void write(const std::string &data)
        {
            _connection.write(data);
        }
    // public:
    //     Response(const std::string &protocol, int status_code, const std::string &status_message, const std::string &body)
    //     {
    //         _out_buffer = protocol + " " + std::to_string(status_code) + " " + status_message + "\r\n";
    //         _out_buffer += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    //         _out_buffer += "\r\n";
    //         _out_buffer += body;
    //     }
    //     std::string _out_buffer;
    private:
        Connection &_connection;
};