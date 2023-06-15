#include <stack>

// todo tele netstat established and connections count
// todo check netstat browser TIME_WAIT
class Webserver
{

private:

typedef std::map<int, Connection> t_connections;
typedef t_connections::iterator t_connections_it;
t_connections connections;

Configuration _configuration;

std::vector<AMiddleware *> _middlewares;

typedef std::map<int, int> t_ports;
typedef t_ports::iterator t_ports_it;
t_ports _listen_ports;

public:
    Webserver(const std::string &config_path): _configuration(config_path)
    {
        setup_middleware_pipeline();
    }

void setup_middleware_pipeline()
{
 
    //pipeline
    _middlewares.push_back(Singleton<CheckMethod>::get_instance());
    // _middlewares.push_back(Singleton<indexfile>::get_instance());
    // _middlewares.push_back(Singleton<Session>::get_instance()); // bonus
    // _middlewares.push_back(Singleton<Upload>::get_instance()); // /upload
    _middlewares.push_back(Singleton<DirectoryListing>::get_instance());
    // _middlewares.push_back(Singleton<cgi>::get_instance()); // login.py
    _middlewares.push_back(Singleton<StaticFile>::get_instance()); // /home.htm
    _middlewares.push_back(Singleton<ErrorPage>::get_instance()); //default action 404
}

void execute_request(Connection &connection)
{
    Request request(connection._in_buffer);
    Response response(connection, _configuration);

    request.translate_path(_configuration._routes);
   
    for (size_t i = 0; !response.is_ended() && i < _middlewares.size(); ++i)
        _middlewares[i]->execute(request, response);

    connection._in_buffer.clear();
}

int accept_with_select(void)
{
    fd_set readfds, writefds, exceptfds; //fd 0-1023 1024
    struct timeval timeout = {5,0};
    int max_fd, number_of_fd, fd;
    struct sockaddr_in address;
    int length;
    std::stack<int> to_remove;
    int keep_alive_count;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);

        for (t_ports_it it = _listen_ports.begin(); it != _listen_ports.end(); ++it)
        {
            FD_SET(it->second, &readfds); // multi site => multi port
            max_fd = std::max(max_fd, it->second);
        }

        keep_alive_count = 0;
        for (t_connections_it it = connections.begin(); it != connections.end(); ++it)
        {
            fd = it->second.fd();
            if (fd > max_fd)
                max_fd = fd;
            if (it->second.status() == READING)
            {
                FD_SET(fd, &readfds);
                keep_alive_count += it->second._in_buffer.empty();
            }
            else  if (it->second.status() == SENDING)
                FD_SET(fd, &writefds);
            FD_SET(fd, &exceptfds);
        }

        std::cout << "connections " << connections.size()  << std::endl;
        // to timeout keep-alive connections
        if (keep_alive_count)
            number_of_fd = select( max_fd + 1 , &readfds , &writefds , &exceptfds, &timeout);
        else
            number_of_fd = select( max_fd + 1 , &readfds , &writefds , &exceptfds, NULL);
        if (number_of_fd < 0)
        {
            perror("select failed");
        }
        else if (number_of_fd == 0)
        {
            // remove timed-out keep-alive connections
            for (t_connections_it it = connections.begin(); it != connections.end(); ++it)
            // for (std::size_t i = 0; i < connections.size(); ++i)
            {
                // Connection &second = connections[i];
                if (it->second.status() == READING && it->second._in_buffer.empty())
                {
                    // std::cout << "timeout " << it->first << "|" << it->second.fd() << std::endl;
                    to_remove.push(it->first);
                    it->second._close();
                }
            }
            while (!to_remove.empty())
            {
                // std::cout << "remove timeout " << to_remove.top() << std::endl;
                connections.erase(to_remove.top());
                to_remove.pop();
            }
            // throw std::runtime_error(strerror(errno));
            continue;
        }
        for (t_ports_it it = _listen_ports.begin(); it != _listen_ports.end(); ++it)
        {
            if (FD_ISSET(it->second, &readfds))
            {
                --number_of_fd;
                fd = accept(it->second, (struct sockaddr *)&address, (socklen_t*)&length);
                if (fd < 0)
                {
                    perror("accept");
                }
                else
                {
                    fcntl(fd, F_SETFL, O_NONBLOCK); //mac only, can skip for linux
                    connections[fd] = Connection(fd, it->first);
                }
            }
        }
        
        for (t_connections_it it = connections.begin(); number_of_fd && it != connections.end(); ++it)
        {
            fd = it->second.fd();
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
                it->second.transmit();
            }
            // todo OOB
            if (FD_ISSET(fd, &exceptfds))
            {
                --number_of_fd;
                to_remove.push(fd);
                it->second.except();
                std::cout << "remove except fd" << std::endl;
            }
            if (it->second.status() == CLOSED)
                to_remove.push(fd);
        }

        while (!to_remove.empty())
        {
            connections.erase(to_remove.top());
            to_remove.pop();
        }
    }
            
    return (1);
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

    accept_with_select();

    for (std::map<int, int>::iterator it = _listen_ports.begin(); it != _listen_ports.end(); ++it)
    {
        if (it->second == -1)
            continue;
        shutdown(it->second, SHUT_RDWR);
        close(it->second);
    }
}

};
