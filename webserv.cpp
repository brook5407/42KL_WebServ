#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <map>
#include <vector>
#include <fcntl.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <string>
#include <sstream>
#include <stdexcept>

#include "Connection.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Middleware.hpp"


typedef std::map<int, Connection> t_connections;
typedef t_connections::iterator t_connections_it;
t_connections connections;

void execute_request(Connection &connection)
{
    (void)  connection;
    Request request(connection._in_buffer);
    Response response(connection);

    std::vector<Middleware *> middlewares;
    // middlewares.push_back(IndexFile());
    middlewares.push_back(Singleton<StaticFile>::get_instance());
    middlewares.push_back(Singleton<ErrorPage>::get_instance());

    for (size_t i = 0; !response.is_ended() && i < middlewares.size(); ++i)
    {
        std::cout << "executing middleware " << i << std::endl;
        middlewares[i]->execute(request, response);
    }

    // StaticFile().execute(request, response);
}

// todo
// multiple listening sockets

// config [server:port][routes]
// request server1:80/dir1/dir3
// routes [server1|80|/]
// routes [server1|80|/dir1]
// routes [default|80|/dir1/dir2]
// routes [server2:88][/]


int accept_with_select(int listen_socket)
{
    fd_set readfds, writefds, exceptfds;
    // struct timeval timeout = {0, 0};
    int max_fd = listen_socket;
    struct sockaddr_in address;
    int length;
    std::vector<int> to_remove;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    FD_SET(listen_socket, &readfds);

    // process connections
    for (t_connections_it it = connections.begin(); it != connections.end(); ++it)
    {
        std::cout << "request: " << it->second.status() << ", " << it->second._in_buffer.substr(0,14) << std::endl;
        int fd = it->second.fd();
        if (fd > max_fd)
            max_fd = fd;
        if (it->second.status() == READING)
            FD_SET(fd, &readfds);
        else  if (it->second.status() == SENDING)
            FD_SET(fd, &writefds);
        // else
        //     std::cout << "remove done connection" << std::endl;
        // FD_SET(fd, &exceptfds);
    }

//10 server => 10 listening ports

    std::cout << "connections " << connections.size() << std::endl;

    // wait for events
    // fd/socket, 4,5,6, fd,  for(i =0; i < max;i++)
    // 1024 bits/flags 000000000111000_, 1 fd is avaialble
    //=  select ( 6+1, [4, 6], [5]) => 2, [4], [5]
    int number_of_fd = select( max_fd + 1 , &readfds , &writefds , &exceptfds, NULL); // &timeout);
    if (number_of_fd < 0)
        throw std::runtime_error(strerror(errno));
    if (FD_ISSET(listen_socket, &readfds))
    {
        --number_of_fd;
        int data_socket = accept(listen_socket, (struct sockaddr *)&address, (socklen_t*)&length);
        if (data_socket < 0)
        {
            // how to test?
            if  ((errno == ENETDOWN || errno == EPROTO || errno == ENOPROTOOPT || errno == EHOSTDOWN ||
                    // errno == ENONET || 
                    errno == EHOSTUNREACH || errno == EOPNOTSUPP || errno == ENETUNREACH))
            {
                perror("accept");
                return true; // confusing...
            }
            throw std::runtime_error(strerror(errno));
        }
        fcntl(data_socket, F_SETFL, O_NONBLOCK);
        connections[data_socket] = Connection(data_socket);
    }
    
    for (std::map<int, Connection>::iterator it = connections.begin(); number_of_fd && it != connections.end(); ++it)
    {
        int fd = it->second.fd();
        if (FD_ISSET(fd, &readfds))
        {
            --number_of_fd;
            it->second.read();
            if (it->second.status() == READ)
                execute_request(it->second);
        }
        else if (FD_ISSET(fd, &writefds))
        {
            --number_of_fd;
            try
            {
                it->second.transmit();
                if (it->second.status() == SENT || it->second.status() == ERROR)
                {
                    close(fd);
                    to_remove.push_back(fd);
                }
            }
            catch (std::exception &e)
            {
                //eg  Connection reset by peer, Broken pipe
                std::cout << "send exception & remove: " << e.what() << std::endl;
                // it->second.error();
                close(fd);
                to_remove.push_back(fd);
            }
        }
        else if (FD_ISSET(fd, &exceptfds))
        {
            --number_of_fd;
            it->second.except();
        }
    }

    // remove done connections
    for (std::vector<int>::iterator it = to_remove.begin(); it != to_remove.end(); ++it)
        connections.erase(*it);
    return (1);
}

void server(int port = 8888, int back_log = 3)
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
    if (listen(listen_socket, back_log) < 0)
        throw std::runtime_error(strerror(errno));
    while (accept_with_select(listen_socket))
        ;
    shutdown(listen_socket, SHUT_RDWR);
}

int main(int argc, char **argv, char **)
{
signal(SIGPIPE, SIG_IGN);
        if (argc != 2)
            throw std::runtime_error("usage: webserv <port>");
        int port = atoi(argv[1]);
        std::cout << "webserv @ " << port << std::endl;
        server(port);
        return EXIT_SUCCESS;
    try
    {
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
