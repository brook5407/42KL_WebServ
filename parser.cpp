#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "server.hpp"

typedef std::vector<std::string>::iterator conf_t;

std::vector<std::string> split(std::string str, std::string delimiter)
{
    std::vector<std::string> result;
    size_t index;
    while(str.size())
    {
        index = str.find(delimiter[0]);
        for (size_t i = 1; i < delimiter.size(); i++)
        {
            if (index > str.find(delimiter[i]))
                index = str.find(delimiter[i]);
        }
        if(index != std::string::npos )
        {
            if (index != 0)
                result.push_back(str.substr(0,index));
            str = str.substr(index + 1);
        }
        else
        {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

Location *parse_location(conf_t &line_pos, conf_t end)
{
    Location *location = new Location();
    while (line_pos != end)
    {
        std::vector<std::string> line = split(*line_pos, " ;\t\n");
        if (!line.size() || line[0] == "")
        {
            line_pos++;
            continue;
        }
        if (line[0] == "}" && line.size() == 1)
        {
            line_pos++;
            return (location);
        }
        else if (line[0] == "location")
        {
            if (line.size() != 3)
                throw std::invalid_argument("Error: location block requires a path and a block");
            else
                location->setPrefix(line[1]);
        }
        else if (*line_pos->rbegin() != ';')
            throw std::invalid_argument("Error: the line must end with semicolon >> " + *line_pos);
        else if (line[0] == "root")
        {
            if (line.size() != 2)
                throw std::invalid_argument("Error: root block requires a path");
            else
                location->setRoot(line[1]);
        }
        else if (line[0] == "methods")
        {
            if (line.size() < 2)
                throw std::invalid_argument("Error: methods block requires at least one method");
            else
            {
                for (size_t i = 1; i < line.size(); i++)
                    location->setMethod(line[i]);
            }
        }
        else if (line[0] == "index")
        {
            if (line.size() < 2)
                throw std::invalid_argument("Error: index block requires a filename"); 
            else
            {
                for (size_t i = 1; i < line.size(); i++)
                    location->setIndex(line[i]);
            }
        }
        else if (line[0] == "autoindex")
        {
            if (line.size() != 2)
                throw std::invalid_argument("Error: autoindex block requires on or off");
            else
                location->setAutoIndex(line[1]);
        }
        else if (line[0] == "cgi_extensions")
        {
            if (line.size() < 2)
                throw std::invalid_argument("Error: cgi_extensions block requires at least one extension");
            else
                location->setCgiExtension(line[1]);

        }
        else if (line[0] == "return")
        {
            if (line.size() != 3)
                throw std::invalid_argument("Error: return block requires a code and a path");
            else
                location->setRedirection(line[1], line[2]);
        }
        else
            throw std::invalid_argument("Error: unknown block >> " + *line_pos);
        line_pos++;
    }
    return (location);
} 


Server	*parse_server(conf_t &line_pos, conf_t end)
{
    Server *server = new Server();
    while (line_pos != end)
    {
        std::vector<std::string> line = split(*line_pos, " ;\t\n");
        if (!line.size() || line[0] == "")
        {
            line_pos++;
            continue;
        }
        if (line[0] == "}" && line.size() == 1)
            break ;
        else if (line[0] == "location")
        {
            if (line.size() != 3 || line[2] != "{")
                throw std::invalid_argument("Error: wrong location format");
            else
            {
                // line_pos++;
                server->addLocation(parse_location(line_pos, end));
                continue;
            }
        }
        else if (line[0] == "listen")
        {
            if (line.size() != 2)
                throw std::invalid_argument("Error: listen block requires an ip and a port");
            else
            {
                std::vector<std::string> ip_port = split(line[1], ":");
                if (ip_port.size() == 1)
                    throw std::invalid_argument("Error: listen block requires an ip and a port");
                else
                {
                    server->setHost(ip_port[0]);
                    server->setPort(ip_port[1]);
                }
            }
        }
        else if (line[0] == "server_name")
        {
            if (line.size() < 2)
                throw std::invalid_argument("Error: server_name block requires at least one name");
            else
            {
                for (size_t i = 1; i < line.size(); i++)
                    server->setName(line[i]);
            }
        }
        else if (line[0] == "client_max_body_size")
        {
            if (line.size() != 2)
                throw std::invalid_argument("Error: client_max_body_size block requires a size");
            else
                server->setMaxBodySize(line[1]);
        }
        else if (line[0] == "error_page")
        {
            if (line.size() != 3)
                throw std::invalid_argument("Error: error_page block requires a code and a path");
            else
                server->setErrorPage(line[1], line[2]);
        }
        else
            throw std::invalid_argument("Error: unknown block >> " + *line_pos);
        line_pos++;
    }
    std::cout << *server << std::endl; // DEBUG
    return (server);
}

std::vector<Server *>   parse_config(std::vector<std::string> config)
{
    std::vector<Server *>   servers;
    for (conf_t line_pos = config.begin(); line_pos != config.end(); line_pos++)
    {
        std::vector<std::string> line = split(*line_pos, " ;\t\n");
        if (!line.size() || line[0] == "")
        {
            line_pos++;
            continue;
        }
        if (line[0] == "server")
        {
            if (line.size() != 2 || line[1] != "{")
                throw std::invalid_argument("Error: wrong server format");
            else
            {
                line_pos++;
                servers.push_back(parse_server(line_pos, config.end()));
                continue;
            }
        }
        else
            throw std::invalid_argument("Error: unknown block >> " + *line_pos);
    }
    return (servers);
}

int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
        {
            std::cout << "Usage: ./webserv <config_file>" << std::endl;
            return (1);
        }
        std::ifstream   file(argv[1]);
        if (file)
        {
            std::stringstream   buffer;
            buffer << file.rdbuf();
            file.close();
            std::string         str = buffer.str();
            std::vector<std::string>    config = split(str, "\n");
            std::vector<Server *>   _servers = parse_config(config); //store this variable in webserver class
        }
        else
            throw std::invalid_argument("Error: cannot open file");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
