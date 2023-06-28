#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Configuration.hpp"
#include <map>
#include <string>

class Request
{
public:
    Request(const std::string &request);

    //todo: throw httpexception which handled by middleware pipeline
    //todo: handle //, /. and /.. in path
    std::string _method;
    std::string _uri;
    std::string _search;
    std::string _script_name;
    std::map<std::string, std::string> _headers;
    std::size_t _content_length;
    bool is_ready;
    std::string _body;

    Server *_server_config;
    Location *_location_config;
};

#endif