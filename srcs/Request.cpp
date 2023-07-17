#include "Request.hpp"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <fstream>

const std::string   &Request::get_body(void) const { return _body; }
const std::string   &Request::get_method(void) const { return _method; }
const std::string   &Request::get_uri(void) const { return _uri; }
const std::string   &Request::get_query_string(void) const { return _search; }
const std::string   &Request::get_translated_path(void) const { return _script_name; }
std::size_t         Request::get_body_length(void) const { return _body_length; }
const Server        &Request::get_server_config(void) const { return *_server_config; };
const Location      &Request::get_location_config(void) const { return *_location_config; };
bool                Request::is_ready(void) const { return _is_ready; }
void                Request::set_translated_path(const std::string &value) { _script_name = value; }


static std::string url_decode(const std::string &value);

// todo: support both \n and \r\n?
Request::Request(const std::vector<Server> &configs, std::string &buffer)
: _buffer(buffer),  _body_length(0), _is_ready(false)
{
    static Location default_location;
    this->_server_config = &configs.at(0); //first server config as default
    this->_location_config = &default_location; // will be method-not-allowed
    default_location.setMethod("GET"); // use 404 instead of method-not-allowed

    parse_request(buffer);
    if (_is_ready)
    {
        find_server_config(configs);
        find_location_config();
    }
}

void Request::parse_request(const std::string &buffer)
{
    const std::size_t pos_header_end = buffer.find("\r\n\r\n");
    if (pos_header_end == std::string::npos)
        return ;

    // quicker chunked request completeness checker
    if (buffer.size() > pos_header_end + 4
        && buffer.find("Transfer-Encoding: chunked") != std::string::npos
        && buffer.find("\r\n0\r\n\r\n", buffer.size() - 7) == std::string::npos)
        return;

    // todo 400 & close connection
    if (!parse_request_line(buffer))
        return;

    parse_headers(buffer, pos_header_end);

    if (_headers.count("Content-Length"))
    {
        // todo error handler
        _body_length = std::atoi(_headers["Content-Length"].c_str());
        if (_body_length > buffer.size() - pos_header_end - 4)
            return; // stop incomplete body for more recv()
        _body = buffer.substr(pos_header_end + 4, _body_length);
    }
    else if (_headers.count("Transfer-Encoding"))
    {
        if (_headers["Transfer-Encoding"] != "chunked")
        {
            std::cout << "invalid Transfer-Encoding " << _headers["Transfer-Encoding"] << std::endl;
            return;
        }
        if (buffer.find("\r\n0\r\n\r\n", buffer.size() - 7) == std::string::npos)
        {
            return;
        }
        parse_chunked_body(buffer, pos_header_end);
    }
    _is_ready = true;
}

bool Request::parse_request_line(const std::string &buffer)
{
    std::string protocol;
    std::istringstream iss(buffer);

    iss >> _method >> _uri >> protocol;
    if (iss.fail() || protocol != "HTTP/1.1")
        return false;
    size_t pos = _uri.find('?');
    if (pos != std::string::npos)
    {
        if (pos + 1 < _uri.size())
            _search = _uri.substr(pos + 1);
        _uri.erase(pos);
    }
    return true;
}

void Request::parse_headers(const std::string &buffer, const std::size_t pos_header_end)
{
    std::string key, value;
    std::size_t pos_start = buffer.find("\r\n") + 2; // end of first buffer line
    std::size_t pos_end;

    while ((pos_end = buffer.find("\r\n", pos_start)) <= pos_header_end)
    {
        std::size_t pos_key_end = buffer.find(": ", pos_start);
        // key not found
        if (pos_key_end == std::string::npos || pos_key_end + 2 >= pos_end)
        {
            // has previous key
            if (key.size())
            {
                // skip leading spaces of multi-line header value
                while (pos_start < pos_end && (buffer[pos_start] == ' ' || buffer[pos_start] == '\t'))
                    ++pos_start;
                // found multi-line header value
                if (pos_start < pos_end)
                {
                    value = buffer.substr(pos_start, pos_end - pos_start);
                    _headers[key] += ' ' + value;
                }
            }
            continue;
        }
        key = buffer.substr(pos_start, pos_key_end - pos_start);
        value = buffer.substr(pos_key_end + 2, pos_end - pos_key_end - 2);
        _headers[key] = value;
        pos_start = pos_end + 2; // next line begin after \r\n
        // std::cout << "header " << key << ":" << value << std::endl;
    }
}

void Request::parse_chunked_body(const std::string &buffer, const std::size_t pos_header_end)
{
    const std::string hex = "0123456789ABCDEF";
    int chunk_length = 0;

    for (std::size_t pos = pos_header_end + 4, val; pos < buffer.size();)
    {
        val = hex.find(std::toupper(buffer[pos]));
        if (val != std::string::npos)
        {
            chunk_length = chunk_length * 16 + val;
            ++pos;
            continue;
        }

        if (buffer.find("\r\n", pos) != pos)
        {
            std::cout << "invalid chunk size" << std::endl;
            return;
        }
        pos += 2;
        if (chunk_length)
            _body += buffer.substr(pos, chunk_length);
        pos += chunk_length;
        if (buffer.find("\r\n", pos) != pos)
        {
            std::cout << "invalid chunk data" << std::endl;
            return;
        }
        pos += 2;
        if (chunk_length == 0)
            break;
        chunk_length = 0;
    }
    _body_length = _body.size();
    // std::ofstream ofs("chunked.txt", std::ios::out | std::ios::trunc | std::ios::binary);
    // ofs << body;
}


void Request::find_server_config(const std::vector<Server> &server_configs)
{
    if (this->_headers.count("Host"))
    {
        const std::size_t pos = this->_headers["Host"].find(":");
        const std::string host = this->_headers["Host"].substr(0, pos);
        size_t port = 80; //default
        if (pos < this->_headers["Host"].size() - 2)
            port = atoi(this->_headers["Host"].substr(pos + 1).c_str());
        //todo handle possible malform host:port
        for (size_t i = 0; i < server_configs.size(); i++)
        {
            const Server &conf = server_configs[i];
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
}

void Request::find_location_config(void)
{
    const std::vector<Location> &locations = _server_config->getRoutes();
    std::string route = this->_uri; //eg /dir/index.html
    std::size_t pos;
    while (1)
    {
        for (size_t i = 0; i < locations.size(); ++i)
        {
            if (locations[i].getPrefix() == route || locations[i].getPrefix() == route + "/")
            {
                if (locations[i].checkRedirection() && locations[i].getPrefix() !=  this->_uri)
                    continue;
    
                this->_location_config = &locations[i];
                this->_script_name = this->_uri;
                pos = route.find('/');
                if (pos != std::string::npos) //todo route always starts with /
                {
                    this->_script_name.erase(0, route.size()); // + 1);
                    // this->_script_name = this->_script_name.substr(route.size() - pos);
                }
                this->_script_name = this->get_location_config().getRoot() + this->_script_name;
                while (this->_script_name.find("/..") != std::string::npos)
                    this->_script_name.replace(this->_script_name.find("/.."), 2, "/");
                while (this->_script_name.find("//") != std::string::npos)
                    this->_script_name.replace(this->_script_name.find("//"), 2, "/");

                _script_name = url_decode(_script_name);

                // std::cout << this->_uri << " => " << this->_script_name
                //     << " route:" << route
                //     << " prefix:" << locations[i].getPrefix()
                //     << " location:" << locations[i]
                //     << std::endl;
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

const std::string &Request::get_header(const std::string &key) const
{
    static std::string empty_string;
    try
    {
        return _headers.at(key);
    }
    catch (...)
    {
        return empty_string;
    }
}

const Request::t_headers &Request::get_headers(void) const
{
    return _headers;
}
