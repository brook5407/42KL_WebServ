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
            // std::cout << "header " << key << " " << std::endl;
        }
        std::cout << "request " << _method << " " <<  _uri << " " << _protocol << std::endl;
    }
    std::string translate_path(const t_configs &routes)
    {
        std::string route = _headers["Host"] + _uri; // localhost:8080/dir/index.html

        //expects:
        //uri localhost:8080/purple.png =>  / route
        //uri localhost:8080/production or localhost:8080/production/ => /production route
        while (!routes.empty() && routes.count(route) == 0)
        {
            std::size_t pos = route.find_last_of('/');
            if (pos == std::string::npos)
                break;
            route = route.substr(0, pos);
        }
        if (routes.count(route) == 0)
            route = "default";
        std::cout << "config route " << route << " for " << _headers["Host"] << _uri << std::endl;
        _script_name = route;
        return _script_name;
    }
    std::string _method;
    std::string _uri;
    std::string _protocol;
    std::string _search;
    std::string _script_name;
    std::map<std::string, std::string> _headers;
    std::map<std::string, std::string> *_config;
};
