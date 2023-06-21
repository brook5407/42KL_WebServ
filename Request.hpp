#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstdlib>

class Request
{
public:
    //todo: throw httpexception which handled by middleware pipeline
    //todo: handle //, /. and /.. in path
    Request(const std::string &request)
        : _method(""), _uri(""), _search(""), _headers(), _content_length(0), is_ready(false), _body("")
    {
        const std::size_t pos_header_end = request.find("\r\n\r\n");
        if (pos_header_end == std::string::npos)
            return ;
            // throw std::runtime_error("missing header end");

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
            // std::cout << "content length " << _content_length
            //         << "------------------" << std::endl
            //         << request.substr(pos_header_end + 4, _content_length) << std::endl
            //         << "=================" << std::endl;
            if (_content_length > request.size() - pos_header_end - 4)
                return; // stop incomplete body for more recv()
            _body = request.substr(pos_header_end + 4, _content_length);
        }
        else if (_headers.count("Transfer-Encoding"))
        {
            if (_headers["Transfer-Encoding"] != "chunked")
            {
                std::cout << "invalid Transfer-Encoding " << _headers["Transfer-Encoding"] << std::endl;
                return;
            }
            // std::cout << "xxxxxxxxxxxxxxxxxxxxxxxxxx" << std::endl << request.substr(pos_header_end + 4) << std::endl << "xxxxxxxxxxxxxxxxxxxxxxxxxx" << std::endl;
            int length = 0;
            std::string body;
            bool complete = false;
            const std::string hex = "0123456789ABCDEF";
            for (pos = pos_header_end + 4; pos < request.size();)
            {
                std::size_t v = hex.find(std::toupper(request[pos]));
                if (v != std::string::npos)
                {
                    length = length * 16 + v;
                    ++pos;
                }
                else
                {
                    if (request.find("\r\n", pos) != pos)
                    {
                        std::cout << "invalid chunk size "
                            << std::endl;
                            // << length << " pos " << request.find("\r\n", pos) << " vs "  << pos << " ch " << request[pos] << std::endl;
                        return;
                    }
                    pos += 2;
                    if (length)
                        body += request.substr(pos, length);
                    pos += length;
                    if (request.find("\r\n", pos) != pos)
                    {
                        std::cout << "invalid chunk data"
                            << std::endl;
                            //  << "=========" << std::endl << body << "=============" << std::endl << body.size() << " len "  << length << std::endl;
                            // std::cout << "check " << pos << " vs " << request.find("\r\n", pos) << " ch: " << request[pos] << std::endl;
                        return;
                    }
                    pos += 2;
                    if (length == 0)
                    {
                        complete = true;
                        break;
                    }
                    length = 0;
                }
            }
            if (!complete)
            {
                std::cout << "incomplete chunked request " << std::endl;
                return;
            }
            // std::cout << "chunked request body "
            //     << "-----------------------" << std::endl 
            //     << body << std::endl
            //     << "=====================" << std::endl;
            // save chunk to file, replace all
            // std::ofstream ofs("chunked.txt", std::ios::out | std::ios::trunc | std::ios::binary);
            // ofs << body;
            _body = body;
        }
        is_ready = true;
    }
    std::string translate_path(Configuration &configuration) //todo? const t_configs &
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
    bool is_ready;
    std::string _body;
};

#endif
