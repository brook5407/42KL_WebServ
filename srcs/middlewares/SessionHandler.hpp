#ifndef SESSIONHANDLER_HPP
# define SESSIONHANDLER_HPP

#include "Middleware.hpp"

class SessionHandler : public Middleware
{
    public:
        void execute(Request &req, Response &res);
        const std::string &get_session();
        const std::string &get_session_id() const;
        std::string extract_cookie(const std::string &cookies, const std::string &cookie_name);
        void set_session(const std::string &session_id, const std::string &new_value);
        bool parse_session(const std::string &session_id, const std::string &cgi_response_header);

        std::string current_session_ID;
        std::map<std::string, std::string> _session;
};

#endif