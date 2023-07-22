#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Server.hpp"
# include <map>
# include <vector>
# include <string>

class Request
{
public:
    typedef std::map<std::string, std::string> t_headers;

    Request(const std::vector<Server> &servers, std::string &buffer);

    const std::string   &get_header(const std::string &key) const;
    const t_headers     &get_headers(void) const;
    const std::string   &get_body(void) const;
    const std::string   &get_method(void) const;
    const std::string   &get_uri(void) const;
    const std::string   &get_query_string(void) const;
    const std::string   &get_translated_path(void) const;
    void                set_translated_path(const std::string &value);
    std::size_t         get_body_length(void) const;
    const Server        &get_server_config(void) const;
    const Location      &get_location_config(void) const;
    bool                is_ready(void) const;

private:
    std::string         &_buffer;
    const Server        *_server_config;
    const Location      *_location_config;
    t_headers           _headers;
    std::string         _body;
    std::size_t         _body_length;
    std::string         _method;
    std::string         _uri;
    std::string         _search;
    std::string         _script_name;
    bool                _is_ready;

    Request(void);
    void parse_request(const std::string &buffer);
    bool parse_request_line(const std::string &buffer);
    void parse_headers(const std::string &buffer, const std::size_t pos_header_end);
    void parse_chunked_body(const std::string &buffer, const std::size_t pos_header_end);
    void find_server_config(const std::vector<Server> &servers);
    void find_location_config(void);
};

#endif
