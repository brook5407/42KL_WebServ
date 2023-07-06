#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#define LISTEN_BACKLOG 128
#define POLL_TIMEOUT_SEC 5
#define CONNECTION_TIMEOUT_SEC 300

#include "Pipeline.hpp"
#include "Connection.hpp"
#include "Server.hpp"
#include <list>

// todo tele netstat established and connections count
// todo check netstat browser TIME_WAIT
class Webserver
{
public:
    Webserver(const std::string &config_filepath);
    ~Webserver(void);
    void loop(void);

private:
    typedef std::list<int> t_listen_sockets;
    
    static Webserver *_instance;

    Pipeline _pipeline;
    std::vector<Server> _serverConfigs;

    int _create_listen_socket(const char *address, int port);
    void _loop_sockets(t_listen_sockets &);
    void _process_request(Connection &connection);
    static void _on_cgi_exit(int signal);

};

#endif
