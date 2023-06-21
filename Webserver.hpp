#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#define LISTEN_BACKLOG 8
#define POLL_TIMEOUT_SEC 5
#define CONNECTION_TIMEOUT_SEC 30

#include "Configuration.hpp"
#include "Middleware.hpp"
#include "Connection.hpp"
#include "Server.hpp"
#include <list>
#include <map>

// todo tele netstat established and connections count
// todo check netstat browser TIME_WAIT
class Webserver
{
public:
    Webserver(const std::string &config_filepath);
    ~Webserver(void);
    void loop(void);

private:
    static Webserver *_instance;
    typedef std::list<Connection> t_connections;
    typedef std::map<int, int> t_listen_port_fd;

    Configuration _configuration;
    Pipeline _pipeline;
    std::vector<Server *> _serverConfigs;

    int _create_listen_socket(int port);
    void _loop_sockets(t_listen_port_fd);
    void _process_request(Connection &connection);
    static void _on_cgi_exit(int signal);
};

#endif
