#include "Webserver.hpp"
#include "ConfigParser.hpp"

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <list>
#include <map>
#include <algorithm>

Webserver *Webserver::_instance = NULL;
int g_exit = false;
void _request_exit(int)
{
    g_exit = true;
    std::cout << "exit signal received" << std::endl;
}

Webserver::Webserver(const std::string &config_filepath)
{
    ConfigParser    parser(config_filepath);
    _serverConfigs = parser.getServers();
    _instance = this;
}

Webserver::~Webserver(void)
{
    _instance = NULL;
}

void Webserver::find_config(Request &request)
{
    request._server_config = &_serverConfigs[0]; //first server config as default
    if (request._headers.count("Host"))
    {
        const std::size_t pos = request._headers["Host"].find(":");
        const std::string host = request._headers["Host"].substr(0, pos);
        size_t port = 80; //default
        if (pos < request._headers["Host"].size() - 2)
            port = atoi(request._headers["Host"].substr(pos + 1).c_str());
        //todo handle possible malform host:port
        for (size_t i = 0; i < _serverConfigs.size(); i++)
        {
            Server &conf = _serverConfigs[i];
            if (conf.getPort() != port)
                continue;
            if (std::find(conf.getNames().begin(), conf.getNames().end(), host) != conf.getNames().end())
            {
                request._server_config = &conf;
                // std::cout << "found server config for " << host << ":" << port << std::endl;
                break;
            }
        }
    }
    std::vector<Location> &locations = request._server_config->getRoutes();
    std::string route = request._uri; //eg /dir/index.html
    std::size_t pos;
    while (1)
    {
        for (size_t i = 0; i < locations.size(); ++i)
        {
            if (locations[i].getPrefix() == route || locations[i].getPrefix() == route + "/")
            {
                request._location_config = &locations[i];
                request._script_name = request._uri;
                pos = route.find('/');
                if (pos != std::string::npos) //todo route always starts with /
                {
                    request._script_name.erase(0, route.size()); // + 1);
                    // request._script_name = request._script_name.substr(route.size() - pos);
                }
                request._script_name = request._location_config->getRoot() + request._script_name;
                while (request._script_name.find("/..") != std::string::npos)
                    request._script_name.replace(request._script_name.find("/.."), 2, "/");
                while (request._script_name.find("//") != std::string::npos)
                    request._script_name.replace(request._script_name.find("//"), 2, "/");

                std::cout << request._uri << " => " << request._script_name << std::endl;
                // std::cout << "found location config for " << route << " | "
                //      << *request._location_config << std::endl;
                return;
            }
        }

        pos = route.find_last_of('/');
        if (pos == std::string::npos)
            break;
        route.erase(pos);
    }
    // default to Location /
}

void Webserver::_process_request(Connection &connection)
{
    #if __APPLE__ && __MACH__
    #else
    if (connection._in_buffer.find("GET / ") == 0 && connection.status() == READING
        && connection._in_buffer.find("\r\n\r\n", connection._in_buffer.size()-4) != std::string::npos
        && connection._in_buffer.find("Go-http-client") != std::string::npos)
    {
        Response(connection, _configuration).send_content(200, "OK");
        return;
    }
    #endif
    Request request(connection._in_buffer); // parser
    if (!request.is_ready)
        return;
    find_config(request);
    Response response(connection, _configuration);
   _pipeline.execute(request, response);
}

// reverse of execution, due to loop
// 1) cleanup connections
// 2) send data
// 3) read & process data (eg parse & do request)
// 4) accept new connection
void Webserver::_loop_sockets(t_listen_sockets &listen_sockets)
{
    typedef std::list<Connection> t_connections;
    fd_set readfds, writefds, exceptfds; //fd 0-1023 1024
    struct timeval timeout;
    int max_fd, number_of_fd, fd;
    struct sockaddr_un address;
    int length;
    t_connections connections;

    while (1)
    {
        max_fd = 0;
        timeout.tv_sec = POLL_TIMEOUT_SEC;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);

        for (t_listen_sockets::iterator it = listen_sockets.begin(); it != listen_sockets.end(); ++it)
        {
            FD_SET(*it, &readfds);
            max_fd = std::max(max_fd, *it);
        }

        // just set response to 502
        // select blocks, this function is read only after select times out
        // for convenience is_timeout and select timeout should be the same
        for (std::list<CGI>::iterator it = Singleton<CgiRunner>::get_instance()->_CGI.begin(); it != Singleton<CgiRunner>::get_instance()->_CGI.end();)
        {
            if (it->is_timeout(300))
            {
                it->_response.send_content(502, "Process has timed out");
                kill(it->child_pid, SIGKILL);
                it = Singleton<CgiRunner>::get_instance()->_CGI.erase(it);
            }
            else
                ++it;
        }
    
        for (t_connections::iterator it = connections.begin(); it != connections.end();)
        {
            if (it->status() == CLOSED || it->is_timeout(CONNECTION_TIMEOUT_SEC)) //30 at29
            {
                it = connections.erase(it);
                continue;
            }

            max_fd = std::max(max_fd, it->fd());
            if (it->status() == SENDING)
                FD_SET(it->fd(), &writefds);
            else if (it->status() == READING)
                FD_SET(it->fd(), &readfds);
            FD_SET(it->fd(), &exceptfds);
            ++it;
        }

        // std::cout
        //     << "connections " << connections.size() 
        //     << " max:" << max_fd 
        //     << " cgi: " << Singleton<CgiRunner>::get_instance()->_CGI.size() 
        //     << std::endl;
        number_of_fd = select(max_fd + 1 , &readfds , &writefds , &exceptfds, 
                            connections.size()? &timeout: NULL);
        if (g_exit) {
            std::cout << "bye" << std::endl;
            break;
        }
        // log_if_errno(number_of_fd, "select failed");
        if (number_of_fd < 0)
            perror("select failed");
        if (number_of_fd <= 0)
            continue;
        for (t_connections::iterator it = connections.begin(); it != connections.end(); ++it)
        {
            if (FD_ISSET(it->fd(), &writefds))
                it->transmit();
            else if (FD_ISSET(it->fd(), &readfds))
                it->read(), _process_request(*it);
            else if (FD_ISSET(it->fd(), &exceptfds))
                it->except();
        }
        for (t_listen_sockets::iterator it = listen_sockets.begin(); it != listen_sockets.end(); ++it)
        {
            if (!FD_ISSET(*it, &readfds))
                continue;
            length = sizeof(address);
            fd = accept(*it, (struct sockaddr *)&address, (socklen_t*)&length);
            //log_if_errno(fd, "accept failed");
            if (fd < 0)
            {
                perror("accept failed");
                std::cout << "accept from " << *it << std::endl;
                continue;
            }
            #if __APPLE__ && __MACH__
            fcntl(fd, F_SETFL, O_NONBLOCK);
            #endif
            connections.push_back(Connection(fd));
            // std::cout << "accepted " << connections.back() << std::endl;
        }
    }
}

int Webserver::_create_listen_socket(const char *address_str, int port)
{
    int listen_socket;
    int opt = 1;
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    if (inet_pton(AF_INET, address_str, &(address.sin_addr)) <= 0)
        throw std::runtime_error(strerror(errno));

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
        throw std::runtime_error(strerror(errno));
    // avoid bind error: Address already in use due to TIME_WAIT
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)  
        throw std::runtime_error(strerror(errno));
    if (bind(listen_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
        throw std::runtime_error(strerror(errno));
    if (listen(listen_socket, LISTEN_BACKLOG) < 0)
        throw std::runtime_error(strerror(errno));
    return (listen_socket);
}

// todo from pid get CGI object & connection object.
//   maybe map<pid, object>
// verify kill server, kill cgi
// remove done cgi (as well as destroy)
void Webserver::_on_cgi_exit(int)
{
    int status;
    pid_t pid;
    if (_instance == NULL)
    {
        std::cout << "Server exited" << std::endl;
    }
    CgiRunner  &runner = *(Singleton<CgiRunner>::get_instance());
    std::list<CGI>::iterator it;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        std::cout << "pid " << pid << " exited with status " << status << std::endl;

        for (it = runner._CGI.begin(); it != runner._CGI.end(); ++it)
        {
            if (it->child_pid == pid)
            {
                //todo: ctrl+c curl trigger heap-use-after-free, due to removed Connection
                //todo: delay and split reading stdout for responsiveness
                it->response(); //todo check error 500 if error/empty, timeout not here but loop_soket()
                runner._CGI.erase(it); // remove done CGI
                break;
            }
        }
    }
}

void Webserver::loop(void)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGINT, _request_exit); // ctrl-c
    signal(SIGPIPE, SIG_IGN); // ignore broken pipe
    signal(SIGCHLD, _on_cgi_exit); // waitpid

    t_listen_sockets listen_sockets;
    for (size_t i = 0; i < _serverConfigs.size(); ++i)
    {
        const int port = _serverConfigs[i].getPort();
        std::string address = _serverConfigs[i].getHost();
        // todo skip duplicate ports, instead of error, so multiple servers on same port
        std::cout << "Webserver@" << address << ":" << port << std::endl;
        listen_sockets.push_back(_create_listen_socket(address.c_str(), port));
    }
    _loop_sockets(listen_sockets);
    for (t_listen_sockets::iterator it = listen_sockets.begin(); it != listen_sockets.end(); ++it)
        close(*it);
    // kill all cgi
}
