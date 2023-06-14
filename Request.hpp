class Request
{
public:
    Request(const std::string &request)
    {
        std::istringstream iss(request);
        iss >> _method >> _uri >> _protocol;
        size_t pos = _uri.find("?");
        if (pos != std::string::npos)
        {
            _uri = _uri.substr(0, pos);
            if (pos + 1 < _uri.size())
                _search = _uri.substr(pos + 1);
        }
        pos = request.find("\r\n\r\n");
        // if (pos == std::string::npos)
        //     return ;
        std::string headers = request.substr(0, pos);
        while ((pos = headers.find("\r\n")) != std::string::npos)
        {
            std::string header = headers.substr(0, pos);
            headers = headers.substr(pos + 2);
            pos = header.find(": ");
            if (pos == std::string::npos)
                continue ;
            std::string key = header.substr(0, pos);
            std::string value = header.substr(pos + 2);
            _headers[key] = value;
        }
        std::cout << "request " << _method << " " <<  _uri << " " << _protocol << std::endl;
    }
    std::string _method;
    std::string _uri;
    std::string _protocol;
    std::string _search;
    std::map<std::string, std::string> _headers;
    std::map<std::string, std::string> *_config;
};
