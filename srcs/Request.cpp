#include "Request.hpp"
#include <iostream>
#include <cstdlib>
#include <algorithm>

static std::string url_decode(const std::string &value);

static bool parse_request_line(
    const std::string &request, std::string &method, std::string &uri, std::string &search)
{
    std::string protocol;
    std::istringstream iss(request);
    iss >> method >> uri >> protocol;
    if (iss.fail() || protocol != "HTTP/1.1")
        return false;
    size_t pos = uri.find('?');
    if (pos != std::string::npos)
    {
        if (pos + 1 < uri.size())
            search = uri.substr(pos + 1);
        uri = uri.substr(0, pos);
    }
    return true;
}

// todo: support both \n and \r\n?
Request::Request(const std::string &request)
: _method(""), _uri(""), _search(""), _headers(), _content_length(0), is_ready(false), _body("")
{
    const std::size_t pos_header_end = request.find("\r\n\r\n");
    if (pos_header_end == std::string::npos)
        return ;

    // quicker chunked request completeness checker
    if (request.size() > pos_header_end + 4
        && request.find("Transfer-Encoding: chunked") != std::string::npos
        && request.find("\r\n0\r\n\r\n", request.size() - 7) == std::string::npos)
        return;

    // todo 400 & close connection
    if (!parse_request_line(request, _method, _uri, _search))
        return;

    std::size_t pos_start = request.find("\r\n") + 2; // end of first request line
    std::size_t pos_end;
    std::string key, value;
    while ((pos_end = request.find("\r\n", pos_start)) <= pos_header_end)
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
        // std::cout << "header " << key << ":" << value << std::endl;
    }
    // std::cout << "Request: " << _method << "|" <<  _uri << "|" << protocol << std::endl;
    if (_headers.count("Content-Length"))
    {
        // todo error handler
        _content_length = std::atoi(_headers["Content-Length"].c_str());
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
        if (request.find("\r\n0\r\n\r\n", request.size() - 7) == std::string::npos)
        {
            // std::cout << "incomplete chunked request " << std::endl;
            return;
        }
        int chunk_length = 0;
        const std::string hex = "0123456789ABCDEF";
        for (std::size_t pos = pos_header_end + 4, val; pos < request.size();)
        {
            val = hex.find(std::toupper(request[pos]));
            if (val != std::string::npos)
            {
                chunk_length = chunk_length * 16 + val;
                ++pos;
                continue;
            }

            if (request.find("\r\n", pos) != pos)
            {
                std::cout << "invalid chunk size" << std::endl;
                return;
            }
            pos += 2;
            if (chunk_length)
                _body += request.substr(pos, chunk_length);
            pos += chunk_length;
            if (request.find("\r\n", pos) != pos)
            {
                std::cout << "invalid chunk data" << std::endl;
                return;
            }
            pos += 2;
            if (chunk_length == 0)
                break;
            chunk_length = 0;
        }
        _content_length = _body.size();
        // std::ofstream ofs("chunked.txt", std::ios::out | std::ios::trunc | std::ios::binary);
        // ofs << body;
    }
    is_ready = true;
}


void Request::find_location_config(std::vector<Server> &_serverConfigs)
{
    static Location default_location;

    this->_server_config = &_serverConfigs.at(0); //first server config as default
    if (this->_headers.count("Host"))
    {
        const std::size_t pos = this->_headers["Host"].find(":");
        const std::string host = this->_headers["Host"].substr(0, pos);
        size_t port = 80; //default
        if (pos < this->_headers["Host"].size() - 2)
            port = atoi(this->_headers["Host"].substr(pos + 1).c_str());
        //todo handle possible malform host:port
        for (size_t i = 0; i < _serverConfigs.size(); i++)
        {
            Server &conf = _serverConfigs[i];
            if (conf.getPort() != port)
                continue;
            if (std::find(conf.getNames().begin(), conf.getNames().end(), host) != conf.getNames().end())
            {
                this->_server_config = &conf;
                // std::cout << "found server config for " << host << ":" << port << std::endl;
                break;
            }
        }
    }
    std::vector<Location> &locations = this->_server_config->getRoutes();
    std::string route = this->_uri; //eg /dir/index.html
    std::size_t pos;
    while (1)
    {
        for (size_t i = 0; i < locations.size(); ++i)
        {
            if (locations[i].getPrefix() == route || locations[i].getPrefix() == route + "/")
            {
                this->_location_config = &locations[i];
                this->_script_name = this->_uri;
                pos = route.find('/');
                if (pos != std::string::npos) //todo route always starts with /
                {
                    this->_script_name.erase(0, route.size()); // + 1);
                    // this->_script_name = this->_script_name.substr(route.size() - pos);
                }
                this->_script_name = this->_location_config->getRoot() + this->_script_name;
                while (this->_script_name.find("/..") != std::string::npos)
                    this->_script_name.replace(this->_script_name.find("/.."), 2, "/");
                while (this->_script_name.find("//") != std::string::npos)
                    this->_script_name.replace(this->_script_name.find("//"), 2, "/");

                _script_name = url_decode(_script_name);

                std::cout << this->_uri << " => " << this->_script_name << std::endl;
                // std::cout << "found location config for " << route << " | "
                //      << *this->_location_config << std::endl;
                return;
            }
        }

        pos = route.find_last_of('/');
        if (pos == std::string::npos)
            break;
        route.erase(pos);
    }
    // default to Location /
    this->_location_config = &default_location; // will be method-not-allowed
    default_location.setMethod("GET"); // use 404 instead of method-not-allowed
}

std::string url_decode(const std::string &value)
{
    std::string result;
    result.reserve(value.size());
    char ch[] = "FF";
    
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        *ch = value[i];
        
        if (*ch == '%' && (i + 2) < value.size())
        {
            ch[0] = value[i + 1];
            ch[1] = value[i + 2];
            result.push_back(static_cast<char>(strtol(ch, NULL, 16)));
            i += 2;
        }
        else if (*ch == '+')
        {
            result.push_back(' ');
        }
        else
        {
            result.push_back(*ch);
        }
    }
    return result;
}
