class Request
{
public:
    //todo: throw httpexception which handled by middleware pipeline
    //todo: handle //, /. and /.. in path
    Request(const std::string &request)
        : _method(""), _uri(""), _search(""), _headers(), _content_length(0)
    {
        const std::size_t pos_header_end = request.find("\r\n\r\n");
        if (pos_header_end == std::string::npos)
            throw std::runtime_error("missing header end");

        std::string protocol;
        std::istringstream iss(request);
        iss >> _method >> _uri >> protocol;
        // todo: check protocol, throw 400

        size_t pos = _uri.find('?');
        if (pos != std::string::npos)
        {
            _uri = _uri.substr(0, pos);
            if (pos + 1 < _uri.size())
                _search = _uri.substr(pos + 1);
        }

        std::size_t pos_start = request.find("\r\n") + 2; // end of first request line
        std::size_t pos_end;
        std::string key, value;
        while ((pos_end = request.find("\r\n", pos_start)) < pos_header_end)
        {
            std::size_t pos_key_end = request.find(": ", pos_start);
            // key not found
            if (pos_key_end == std::string::npos || pos_key_end + 2 >= pos_end)
            {
                // has previous key
                if (key.size())
                {
                    // skip leading spaces of multi-line header value
                    while (pos_start < pos_end && (request[pos_start] == ' ' || request[pos_start] == '\t'))
                        ++pos_start;
                    // found multi-line header value
                    if (pos_start < pos_end)
                    {
                        value = request.substr(pos_start, pos_end - pos_start);
                        _headers[key] += ' ' + value;
                    }
                }
                continue;
            }
            key = request.substr(pos_start, pos_key_end - pos_start);
            value = request.substr(pos_key_end + 2, pos_end - pos_key_end - 2);
            _headers[key] = value;
            pos_start = pos_end + 2; // next line begin after \r\n
            std::cout << "header " << key << ":" << value << std::endl;
        }
        std::cout << "Request: " << _method << "|" <<  _uri << "|" << protocol << std::endl;
        //todo: default for missing header[Host], consider ":80" as ""
        if (_headers.count("Content-Length"))
        {
            // todo error handler
            _content_length = std::atoi(_headers["Content-Length"].c_str());
            std::cout << "content length " << _content_length << ", " << request << std::endl;
            if (_content_length > request.size() - pos_header_end - 4)
            {
                return ;
            }
        }
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
            std::size_t pos = route.find('/');
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
    std::string _search;
    std::string _script_name;
    t_config *_route;
    std::map<std::string, std::string> _headers;
    std::size_t _content_length;
};
