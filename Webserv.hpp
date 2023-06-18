#include <list>

// todo tele netstat established and connections count
// todo check netstat browser TIME_WAIT
class Webserver
{

private:

// typedef std::map<int, Connection> t_connections;
typedef std::list<Connection> t_connections;
typedef t_connections::iterator t_connections_it;
t_connections connections;

Configuration _configuration;

typedef std::map<int, int> t_ports;
typedef t_ports::iterator t_ports_it;
t_ports _listen_ports;

Pipeline _pipeline;

public:
    Webserver(const std::string &config_path): _configuration(config_path)
    {
        _pipeline.add(Singleton<ErrorPage>::get_instance());
        // _pipeline.add(Singleton<Session>::get_instance()); // send cookie if not set, read cookie if set
        _pipeline.add(Singleton<CheckMethod>::get_instance());
        _pipeline.add(Singleton<Redirect>::get_instance());
        _pipeline.add(Singleton<IndexFile>::get_instance());
        _pipeline.add(Singleton<CgiRunner>::get_instance()); // upload.cgi?
        _pipeline.add(Singleton<DirectoryListing>::get_instance());
        _pipeline.add(Singleton<StaticFile>::get_instance());
    }

void execute_request(Connection &connection)
{
    // assert(connection.status() == READING);

    Request request(connection._in_buffer); // parser
    if (!request.is_ready)
        return;
    Response response(connection, _configuration);

    request.translate_path(_configuration);
   
    _pipeline.execute(request, response);
    
    //todo: part of _pipeline?
    response.end();
    connection._in_buffer.clear();
}

// reverse of execution, due to loop
// 1) cleanup connections
// 2) send data
// 3) read & process data (eg parse & do request)
// 4) accept new connection
void loop_sockets(void)
{
    fd_set readfds, writefds, exceptfds; //fd 0-1023 1024
    struct timeval timeout = {5,0};
    int max_fd, number_of_fd, fd;
    struct sockaddr_in address;
    int length;

    while (1)
    {
        max_fd = 0;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);

        for (t_ports_it it = _listen_ports.begin(); it != _listen_ports.end(); ++it)
        {
            FD_SET(it->second, &readfds); // multi site => multi port
            max_fd = std::max(max_fd, it->second);
        }
        for (t_connections_it it = connections.begin(); it != connections.end();)
        {
            if (it->status() == CLOSED || it->is_timeout(timeout.tv_sec))
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
        if (number_of_fd <= 0)
            continue;
        for (t_connections_it it = connections.begin(); it != connections.end(); ++it)
        {
            if (FD_ISSET(it->fd(), &writefds))
                it->transmit();
            else if (FD_ISSET(it->fd(), &readfds))
                it->read(), execute_request(*it);
            else if (FD_ISSET(it->fd(), &exceptfds))
                it->except();
        }
        for (t_ports_it it = _listen_ports.begin(); it != _listen_ports.end(); ++it)
        {
            if (!FD_ISSET(it->second, &readfds))
                continue;
            fd = accept(it->second, (struct sockaddr *)&address, (socklen_t*)&length);
            //log_if_errno(fd, "accept failed");
            if (fd < 0)
                continue;
            // fcntl(fd, F_SETFL, O_NONBLOCK); //mac only, can skip for linux
            connections.push_back(Connection(fd, it->first));
        }
    }
}

int create_listen_socket(int port, int back_log)
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
    return (listen_socket);
}

void start(int port, int back_log = 8)
{
    _listen_ports[port] = -1;
    _listen_ports[9999] = -1;
    for (std::map<int, int>::iterator it = _listen_ports.begin(); it != _listen_ports.end(); ++it)
    {
        std::cout << "webserv @ " << it->first << std::endl;
        it->second = create_listen_socket(it->first, back_log);
    }
    loop_sockets();
    for (std::map<int, int>::iterator it = _listen_ports.begin(); it != _listen_ports.end(); ++it)
        close(it->second);
}

};
