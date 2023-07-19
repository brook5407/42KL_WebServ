#ifndef SESSIONHANDLER_HPP
# define SESSIONHANDLER_HPP

# define SESSION_COOKIE_NAME "ID"

# include "Middleware.hpp"

class SessionHandler : public Middleware
{
    public:
        void execute(Request &req, Response &res);
        std::string get_session() const;
        const std::string &get_session_id() const;
        bool parse_session(const std::string &session_id, const std::string &cgi_response_header);

    private:
        typedef std::map<std::string, std::string> t_sessions;

        std::string _current_session_ID;
        t_sessions  _sessions;

        void set_session(const std::string &session_id, const std::string &new_value);
        static std::string extract_cookie(const std::string &cookies, const std::string &cookie_name);
};

#endif