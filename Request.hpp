class Request
{
public:
    //todo: handle //, /. and /.. in path
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
    std::string translate_path( Configuration &configuration) //todo? const t_configs &
    {
        // assert(!routes.empty());
        std::string route = _headers["Host"] + _uri; // localhost:8080/dir/index.html
        // search exact (Host) route
        while (configuration._routes.count(route) == 0)
        {
            std::size_t pos = route.find_last_of('/');
            if (pos == std::string::npos)
                break;
            route = route.substr(0, pos);
        }
        // use default route if no route found
        if (configuration._routes.count(route) == 0)
        {
            // redo search route within default routes
            route = configuration._default_server + _uri;
            while (configuration._routes.count(route) == 0)
            {
                std::size_t pos = route.find_last_of('/');
                if (pos == std::string::npos)
                    break;
                route = route.substr(0, pos);
            }
        }
        _script_name = _uri;
        {
            std::size_t pos = route.find_first_of('/');
            if (pos != std::string::npos)
                _script_name = _script_name.substr(route.size() - pos);
        }
        _route = &configuration._routes[route];
        std::cout << "config route " << route << " for " << _headers["Host"] << _uri << std::endl;
        _script_name = configuration._routes[route]["root"] + _script_name;
        while (_script_name.find("/..") != std::string::npos)
            _script_name.replace(_script_name.find("/.."), 2, "/");
        while (_script_name.find("//") != std::string::npos)
            _script_name.replace(_script_name.find("//"), 2, "/");
        std::cout << "path " << _script_name << std::endl;
        return _script_name;
    }
    std::string _method;
    std::string _uri;
    std::string _protocol;
    std::string _search;
    std::string _script_name;
    t_config *_route;
    std::map<std::string, std::string> _headers;
};
