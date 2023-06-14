#include <stack>

class Webserv
{

private:

typedef std::map<int, Connection> t_connections;
typedef t_connections::iterator t_connections_it;
t_connections connections;

typedef std::map<std::string, std::string> t_config;
typedef std::map<std::string, t_config > t_configs;
typedef t_configs::iterator t_configs_it;
t_configs _routes;

std::vector<AMiddleware *> _middlewares;

typedef std::map<int, int> t_ports;
typedef t_ports::iterator t_ports_it;
t_ports _listen_ports;

public:

void init()
{
    //route keys by (hostname, port, uri), uri truncated trailing slash
    _routes["default"] = t_config();
    _routes["localhost:9999"] = t_config(); // {root: "./wwwroot", index: "index.html", cgi: false};
    _routes["127.0.0.1:8888"] = t_config(); // {root: "./wwwroot", index: "index.html", cgi: false};
    _routes["localhost:8888"] = t_config(); // {root: "./wwwroot", index: "index.html", cgi: false};
    _routes["localhost:8888/production"] = t_config(); // {root: "./wwwroot", index: "index.html", cgi: false};
    _routes["localhost:8888/virtual"] = t_config(); // {root: "./wwwroot", index: "index.html", cgi: false};

    _routes["127.0.0.1:8888"]["root"] = "./wwwroot";
    _routes["localhost:8888"]["root"] = "./wwwroot/l8";
    _routes["localhost:9999"]["root"] = "./wwwroot/l9";

    //pipeline
    // _middlewares.push_back(Singleton<MethodFilter>::get_instance());
    // _middlewares.push_back(Singleton<indexfile>::get_instance());
    // _middlewares.push_back(Singleton<Session>::get_instance()); // bonus
    // _middlewares.push_back(Singleton<Upload>::get_instance()); // /upload
    _middlewares.push_back(Singleton<DirectoryListing>::get_instance());
    // _middlewares.push_back(Singleton<cgi>::get_instance()); // login.py
    _middlewares.push_back(Singleton<StaticFile>::get_instance()); // /home.htm
    _middlewares.push_back(Singleton<ErrorPage>::get_instance()); //default action 404
}

t_config *get_config_by_route(Request &request)
{
    std::string route = request._headers["Hostname"] + ":" + request._headers["PORT"] + request._uri; // localhost:8080/dir/index.html

    //expects:
    //uri localhost:8080/purple.png =>  / route
    //uri localhost:8080/production or localhost:8080/production/ => /production route
    while (!_routes.empty() && _routes.count(route) == 0)
    {
        std::size_t pos = route.find_last_of('/');
        if (pos == std::string::npos)
            break;
        route = route.substr(0, pos);
    }
    if (_routes.count(route) == 0)
        route = "default";
    std::cout << "config route " << route << " for " << request._uri << std::endl;
    return &_routes[route];
}

void execute_request(Connection &connection)
{
    Request request(connection._in_buffer);
    request._headers["PORT"] = to_string(connection._server_port); // todo refactor
    request._config = get_config_by_route(request);
    Response response(connection);
   
    for (size_t i = 0; !response.is_ended() && i < _middlewares.size(); ++i)
        _middlewares[i]->execute(request, response);

    connection._in_buffer.clear();
}

// todo
// multiple listening sockets

int accept_with_select(void)
{
    fd_set readfds, writefds, exceptfds; //fd 0-1023 1024
    // struct timeval timeout = {0, 0};
    int max_fd, number_of_fd, fd;
    struct sockaddr_in address;
    int length;
    std::stack<int> to_remove;

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

        for (t_connections_it it = connections.begin(); it != connections.end(); ++it)
        {
            std::cout << "request: " << it->second.status() << ", " << it->second._in_buffer.substr(0,14) << std::endl;
            fd = it->second.fd();
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

        std::cout << "connections " << connections.size() << std::endl;

        number_of_fd = select( max_fd + 1 , &readfds , &writefds , &exceptfds, NULL); // &timeout);
        if (number_of_fd < 0)
            throw std::runtime_error(strerror(errno));
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
                    fcntl(fd, F_SETFL, O_NONBLOCK);
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
            else if (FD_ISSET(fd, &exceptfds))
            {
                --number_of_fd;
                it->second.except();
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

void start(int port, int back_log = 64)
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
