/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/20 10:12:24 by chchin            #+#    #+#             */
/*   Updated: 2023/07/02 16:25:23 by chchin           ###   ########.fr       */
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
        throw ParserError("Cannot open file", configFile);
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
                throw ParserError("Wrong server format", *line_pos);
            else
            {
                line_pos++;
                parseServer(line_pos, config.end());
                continue;
            }
        }
        else
            throw ParserError("Unknown block", *line_pos);
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
                throw ParserError("Wrong location format", *line_pos);
            else
            {
                server.addLocation(parseLocation(line_pos, end));
                continue;
            }
        }
        else if (line[0] == "listen")
        {
            if (line.size() != 2)
                throw ParserError("Listen block requires an ip and a port", *line_pos);
            else
            {
                std::vector<std::string> ip_port = ft_split(line[1], ":");
                if (ip_port.size() == 1)
                    throw ParserError("Listen block requires an ip and a port", *line_pos);
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
                throw ParserError("Server_name block requires at least one name", *line_pos);
            else
            {
                for (size_t i = 1; i < line.size(); i++)
                    server.setName(line[i]);
            }
        }
        else if (line[0] == "client_max_body_size")
        {
            if (line.size() != 2)
                throw ParserError("Client_max_body_size block requires a size", *line_pos);
            else
                server.setMaxBodySize(line[1]);
        }
        else if (line[0] == "error_page")
        {
            if (line.size() != 3)
                throw ParserError("Error_page block requires a code and a path", *line_pos);
            else
                server.setErrorPage(line[1], line[2]);
        }
        else
            throw ParserError("Invalid block", *line_pos);
        line_pos++;
    }
    _servers.push_back(server);
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
                throw ParserError("Location block requires a path and a block", *line_pos);
            else
                location.setPrefix(line[1]);
        }
        else if (*line_pos->rbegin() != ';')
            throw ParserError("Line must end with semicolon", *line_pos);
        else if (line[0] == "root")
        {
            if (line.size() != 2)
                throw ParserError("Root block requires a path", *line_pos);
            else
                location.setRoot(line[1]);
        }
        else if (line[0] == "methods")
        {
            if (line.size() < 2)
                throw ParserError("Methods block requires at least one method", *line_pos);
            else
            {
                for (size_t i = 1; i < line.size(); i++)
                    location.setMethod(line[i]);
            }
        }
        else if (line[0] == "index")
        {
            if (line.size() < 2)
                throw ParserError("Index block requires a filename", *line_pos); 
            else
            {
                for (size_t i = 1; i < line.size(); i++)
                    location.setIndex(line[i]);
            }
        }
        else if (line[0] == "autoindex")
        {
            if (line.size() != 2)
                throw ParserError("Autoindex block requires on or off", *line_pos);
            else
                location.setAutoIndex(line[1]);
        }
        else if (line[0] == "cgi_extensions")
        {
            if (line.size() < 2)
                throw ParserError("Cgi_extensions block requires at least one extension", *line_pos);
            else
            {
                for (size_t i = 1; i < line.size(); i++)
                    location.setCgiExtension(line[i]);
            }
        }
        else if (line[0] == "return")
        {
            if (line.size() != 3)
                throw ParserError("Return block requires a code and a path", *line_pos);
            else
                location.setRedirection(line[1], line[2]);
        }
        else if (line[0] == "add_cgi")
            if (line.size() != 3)
                throw ParserError("Add_cgi block requires cgi extension and a path", *line_pos);
            else
                location.setCgiPath(line[1], line[2]);
        else
            throw ParserError("Invalid block", *line_pos);
        line_pos++;
    }
    return (location);
} 

void ConfigParser::checkServer()
{
    // std::vector<Server>::iterator it1;
    // std::vector<Server>::iterator it2;

    // for (it1 = _servers.begin(); it1 != _servers.end(); it1++)
    // {
    //     for (it2 = _servers.begin(); it2 != _servers.end(); it2++)
    //     {
    //         if (it1 != it2 && (*it1).getHost() == (*it2).getHost() && (*it1).getPort() == (*it2).getPort())
    //             throw std::invalid_argument("Error: two servers cannot have the same ip and port");
    //     }
    // }
}

std::vector<Server> &ConfigParser::getServers()
{
    return (_servers);
}