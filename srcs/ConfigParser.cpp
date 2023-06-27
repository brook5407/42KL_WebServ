/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/20 10:12:24 by chchin            #+#    #+#             */
/*   Updated: 2023/06/25 23:31:57 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

ConfigParser::ConfigParser(std::string configFile)
{
    std::ifstream   file(configFile.c_str());
    if (file)
    {
        std::stringstream   buffer;
        buffer << file.rdbuf();
        file.close();
        std::string str = buffer.str();
        std::vector<std::string> config = ft_split(str, "\r\n");
        parseConfig(config);
    }
    else
        throw std::invalid_argument("Error: cannot open file");
    checkServer();
}

ConfigParser::~ConfigParser()
{
}

void ConfigParser::parseConfig(std::vector<std::string> config)
{
    for (conf_t line_pos = config.begin(); line_pos != config.end(); line_pos++)
    {
        std::vector<std::string> line = ft_split(*line_pos, " ;\t\r\n");
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
                parseServer(line_pos, config.end());
                continue;
            }
        }
        else
            throw std::invalid_argument("Error: unknown block >> " + *line_pos);
    }
}

void ConfigParser::parseServer(conf_t &line_pos, conf_t end)
{
    Server server;
    while (line_pos != end)
    {
        std::vector<std::string> line = ft_split(*line_pos, " ;\t\r\n");
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
                server.addLocation(parseLocation(line_pos, end));
                continue;
            }
        }
        else if (line[0] == "listen")
        {
            if (line.size() != 2)
                throw std::invalid_argument("Error: listen block requires an ip and a port");
            else
            {
                std::vector<std::string> ip_port = ft_split(line[1], ":");
                if (ip_port.size() == 1)
                    throw std::invalid_argument("Error: listen block requires an ip and a port");
                else
                {
                    server.setHost(ip_port[0]);
                    server.setPort(ip_port[1]);
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
                    server.setName(line[i]);
            }
        }
        else if (line[0] == "client_max_body_size")
        {
            if (line.size() != 2)
                throw std::invalid_argument("Error: client_max_body_size block requires a size");
            else
                server.setMaxBodySize(line[1]);
        }
        else if (line[0] == "error_page")
        {
            if (line.size() != 3)
                throw std::invalid_argument("Error: error_page block requires a code and a path");
            else
                server.setErrorPage(line[1], line[2]);
        }
        else
            throw std::invalid_argument("Error: unknown block >> " + *line_pos);
        line_pos++;
    }
    _servers.push_back(server);
    // std::cout << *server << std::endl; // DEBUG
}

Location ConfigParser::parseLocation(conf_t &line_pos, conf_t end)
{
    Location location;
    while (line_pos != end)
    {
        std::vector<std::string> line = ft_split(*line_pos, " ;\t\r\n");
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
                location.setPrefix(line[1]);
        }
        else if (*line_pos->rbegin() != ';')
            throw std::invalid_argument("Error: the line must end with semicolon >> " + *line_pos);
        else if (line[0] == "root")
        {
            if (line.size() != 2)
                throw std::invalid_argument("Error: root block requires a path");
            else
                location.setRoot(line[1]);
        }
        else if (line[0] == "methods")
        {
            if (line.size() < 2)
                throw std::invalid_argument("Error: methods block requires at least one method");
            else
            {
                for (size_t i = 1; i < line.size(); i++)
                    location.setMethod(line[i]);
            }
        }
        else if (line[0] == "index")
        {
            if (line.size() < 2)
                throw std::invalid_argument("Error: index block requires a filename"); 
            else
            {
                for (size_t i = 1; i < line.size(); i++)
                    location.setIndex(line[i]);
            }
        }
        else if (line[0] == "autoindex")
        {
            if (line.size() != 2)
                throw std::invalid_argument("Error: autoindex block requires on or off");
            else
                location.setAutoIndex(line[1]);
        }
        else if (line[0] == "cgi_extensions")
        {
            if (line.size() < 2)
                throw std::invalid_argument("Error: cgi_extensions block requires at least one extension");
            else
                location.setCgiExtension(line[1]);

        }
        else if (line[0] == "return")
        {
            if (line.size() != 3)
                throw std::invalid_argument("Error: return block requires a code and a path");
            else
                location.setRedirection(line[1], line[2]);
        }
        else
            throw std::invalid_argument("Error: unknown block >> " + *line_pos);
        line_pos++;
    }
    return (location);
} 

void ConfigParser::checkServer()
{
    std::vector<Server>::iterator it1;
    std::vector<Server>::iterator it2;
    
    for (it1 = _servers.begin(); it1 != _servers.end(); it1++)
    {
        for (it2 = _servers.begin(); it2 != _servers.end(); it2++)
        {
            if (it1 != it2 && (*it1).getHost() == (*it2).getHost() && (*it1).getPort() == (*it2).getPort())
                throw std::invalid_argument("Error: two servers cannot have the same ip and port");
        }
    }
}

std::vector<Server> &ConfigParser::getServers()
{
    return (_servers);
}