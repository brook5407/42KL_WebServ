#include "Webserver.hpp"
#include "CGIHandler.hpp"

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
#include <set>
#include <fcntl.h>

static bool g_exiting = false;

static void exit_internal_loop(int)
{
    g_exiting = true;
    std::cout << "\rWebserv is stopping..." << std::endl;
}

static void exit_select(int)
{
}

Webserver::Webserver(const std::string &config_filepath, int cgi_timeout):
    _config(config_filepath), // parsing of config file starts here
     _cgi_timeout(cgi_timeout)
{
}

void Webserver::process_request(Connection &connection)
{
    Request request(_config.getServers(), connection._request_buffer); // parsing of http request starts here
    if (request.is_ready())
    {
        Response response(connection);
        _pipeline.execute(request, response);
    }
}

void Webserver::accept_client(int socket_fd)
{
    struct sockaddr_un  address;
    int                 length, fd;

    length = sizeof(address);
    fd = accept(socket_fd, (struct sockaddr *)&address, (socklen_t*)&length); // assign fd for client
    if (fd == -1)
    {
        perror("accept failed");
    }
    else
    {
        fcntl(fd, F_SETFL, O_NONBLOCK);
        _client_connections.push_back(Connection(fd)); // connection created here
    }
}

// reverse of execution, due to loop
// 1) cleanup connections
// 2) send data
// 3) read & process data (eg parse & do request)
// 4) accept new connection
void Webserver::internal_loop(void)
{
    fd_set                      readfds, writefds;
    struct timeval              timeout;
    int                         max_fd, ready_fd;
    t_sockets::iterator         fd_it;
    t_connections::iterator     conn_it;

    while (!g_exiting)
    {
        max_fd = 0;
        timeout.tv_sec = POLL_TIMEOUT_SEC;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        // monitor listening port for new connection
        for (fd_it = _server_sockets.begin(); fd_it != _server_sockets.end(); ++fd_it)
        {
            max_fd = *fd_it;
            FD_SET(*fd_it, &readfds);
        }

        CGIHandler::handle_exit();
        CGIHandler::timeout(_cgi_timeout);

        // remove connection if necessary
        // check client ready for reading or sending
        for (conn_it = _client_connections.begin(); conn_it != _client_connections.end();)
        {
            if (conn_it->status() == DISCONNECTED || conn_it->request_timeout(REQUEST_TIMEOUT_SEC))
            {
                conn_it = _client_connections.erase(conn_it);
            }
            else
            {
                max_fd = std::max(max_fd, conn_it->fd());
                if (conn_it->status() == SENDING)
                    FD_SET(conn_it->fd(), &writefds);
                else if (conn_it->status() == READING)
                    FD_SET(conn_it->fd(), &readfds);
                ++conn_it;
            }
        }

        signal(SIGCHLD, exit_select); // allows exit from select once cgi is done
        ready_fd = select(++max_fd, &readfds, &writefds, NULL, _client_connections.empty()? NULL: &timeout); // null block indefinetly until there is a connection
        signal(SIGCHLD, SIG_DFL);
        if (ready_fd <= 0)
            continue;

        // send data when ready
        // read and process response from client
        for (conn_it = _client_connections.begin(); conn_it != _client_connections.end(); ++conn_it)
        {
            if (FD_ISSET(conn_it->fd(), &writefds))
                conn_it->send_response();
            else if (FD_ISSET(conn_it->fd(), &readfds))
                conn_it->recv_request(), process_request(*conn_it);
        }

        // create connection for new client
        for (fd_it = _server_sockets.begin(); fd_it != _server_sockets.end(); ++fd_it)
        {
            if (FD_ISSET(*fd_it, &readfds))
                accept_client(*fd_it);
        }
    }
}

static int create_listen_socket(const char *address_str, int port)
{
    int                 listen_socket;
    int                 opt = 1;
    struct sockaddr_in  address;

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    if (inet_pton(AF_INET, address_str, &(address.sin_addr)) <= 0)
        throw std::runtime_error(strerror(errno));
    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) // create socket here
        throw std::runtime_error(strerror(errno));
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) // setting opt for sockets
        throw std::runtime_error(strerror(errno));
    if (bind(listen_socket, (struct sockaddr *)&address, sizeof(address)) < 0) // binding the port
        throw std::runtime_error(strerror(errno));
    if (listen(listen_socket, LISTEN_BACKLOG) < 0) // enables the socket to be listened
        throw std::runtime_error(strerror(errno));
    return (listen_socket);
}

void Webserver::init_server_ports(void)
{
    std::stringstream       ss;
    std::set<std::string>   unique_bindings;

    for (size_t i = 0; i < _config.getServers().size(); ++i)
    {
        const int           port = _config.getServers().at(i).getPort();
        const std::string   address = _config.getServers().at(i).getHost();

        ss.str(std::string());
        ss << address << ":" << port;
        if (unique_bindings.insert(ss.str()).second == true)
        {
            std::cout << "Webserver@" << ss.str() << std::endl;
            _server_sockets.push_back(create_listen_socket(address.c_str(), port));
        }
    }
}

void Webserver::loop(void)
{
    signal(SIGPIPE, SIG_IGN); // writing to closed socket, or send(MSG_NOSIGNAL)
    signal(SIGINT, SIG_IGN);
    signal(SIGINT, exit_internal_loop);

    init_server_ports();
    internal_loop();
    for (t_sockets::iterator it = _server_sockets.begin(); it != _server_sockets.end(); ++it)
        close(*it);
}
