#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# define LISTEN_BACKLOG 128
# define POLL_TIMEOUT_SEC 1
# define CONNECTION_TIMEOUT_SEC 300

# include "Pipeline.hpp"
# include "Connection.hpp"
# include "ConfigParser.hpp"
# include <list>

class Webserver
{
    public:
        Webserver(const std::string &config_filepath, int cgi_timeout);
        void loop(void);

    private:
        typedef std::list<int>          t_sockets;
        typedef std::list<Connection>   t_connections;

        ConfigParser    _config;
        Pipeline        _pipeline;
        t_sockets       _server_sockets;
        t_connections   _client_connections;
        int             _cgi_timeout;

        Webserver(void);
        void        init_server_ports(void);
        void        internal_loop(void);
        void        accept_client(int socket_fd);
        void        process_request(Connection &connection);
};

#endif
