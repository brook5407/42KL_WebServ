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

Webserver *Webserver::_instance = NULL;

Webserver::Webserver(const std::string &config_filepath)
{
    _instance = this;
    ConfigParser    parser(config_filepath);
    _serverConfigs = parser.getServers();
    // for (std::vector<Server *>::iterator it = servers.begin(); it != servers.end(); it++)
    //     std::cout << **it << std::endl;
}

Webserver::~Webserver(void)
{
    _instance = NULL;
}

void Webserver::_process_request(Connection &connection)
{
    // assert(connection.status() == READING);

    Request request(connection._in_buffer); // parser
    if (!request.is_ready)
        return;
    Response response(connection, _configuration);
    request.translate_path(_configuration);
    _pipeline.execute(request, response);
    if (Singleton<CgiRunner>::get_instance()->is_CGI)
        return ;
    //todo: part of _pipeline?
    response.end();
    // connection._in_buffer.clear();
}

// reverse of execution, due to loop
// 1) cleanup connections
// 2) send data
// 3) read & process data (eg parse & do request)
// 4) accept new connection
void Webserver::_loop_sockets(t_listen_port_fd listen_port_fd)
{
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

        for (t_listen_port_fd::iterator it = listen_port_fd.begin(); it != listen_port_fd.end(); ++it)
        {
            FD_SET(it->second, &readfds); // multi site => multi port
            max_fd = std::max(max_fd, it->second);
        }

        // loop Singleton<CgiRunner>::get_instance()->_CGI
        // {
        //     if (cgi->is_timeout())
        //     {
        //         response 502 error
        //         remove from _cgi
        //     }
        // }

        // just set response to 502
        // select blocks, this function is read only after select times out
        // for convenience is_timeout and select timeout should be the same
        for (std::list<CGI>::iterator it = Singleton<CgiRunner>::get_instance()->_CGI.begin(); it != Singleton<CgiRunner>::get_instance()->_CGI.end(); ++it)
        {
            if (it->is_timeout(10))
            {
                it->_response.status(502);
                it->_response.write("Process has timed out");
                kill(it->child_pid, SIGKILL);
            }
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

        std::cout << "connections " << connections.size() << " max:" << max_fd << std::endl;
        number_of_fd = select(max_fd + 1 , &readfds , &writefds , &exceptfds, 
                            connections.size()? &timeout: NULL);
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
        for (t_listen_port_fd::iterator it = listen_port_fd.begin(); it != listen_port_fd.end(); ++it)
        {
            if (!FD_ISSET(it->second, &readfds))
                continue;
            length = sizeof(address);
            fd = accept(it->second, (struct sockaddr *)&address, (socklen_t*)&length);
            //log_if_errno(fd, "accept failed");
            if (fd < 0)
            {
                perror("accept failed");
                std::cout << "accept from " << it->second << std::endl;
                continue;
            }
            // fcntl(fd, F_SETFL, O_NONBLOCK); //mac only, can skip for linux
            connections.push_back(Connection(fd, it->first));
        }
    }
}

int Webserver::_create_listen_socket(int port)
{
    int listen_socket;
    int length = sizeof(sockaddr_in);
    int opt = 1;
    struct sockaddr_in address = {};

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
        throw std::runtime_error(strerror(errno));
    // avoid bind error: Address already in use due to TIME_WAIT
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)  
        throw std::runtime_error(strerror(errno));
    if (bind(listen_socket, (struct sockaddr *)&address, length) < 0)
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
                it->response(); //todo check error 500 if error/empty, timeout not here but loop_soket()
                runner._CGI.erase(it); // remove done CGI
                break;
            }
        }
    }
}

void Webserver::loop(void)
{
    // signal(SIGINT, _request_exit); // ctrl-c
    signal(SIGPIPE, SIG_IGN); // ignore broken pipe
    signal(SIGCHLD, _on_cgi_exit); // waitpid

    t_listen_port_fd listen_port_fd;
    for (size_t i = 0; i < _serverConfigs.size(); ++i)
    {
        std::cout << "Webserver listening on port " << _serverConfigs[i]->getPort() << std::endl;
        listen_port_fd[_serverConfigs[i]->getPort()] = _create_listen_socket(_serverConfigs[i]->getPort());
    }
    _loop_sockets(listen_port_fd);
    for (t_listen_port_fd::iterator it = listen_port_fd.begin(); it != listen_port_fd.end(); ++it)
        close(it->second);
    // kill all cgi
}
