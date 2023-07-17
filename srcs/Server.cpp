/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 15:50:21 by chchin            #+#    #+#             */
/*   Updated: 2023/07/16 01:01:39 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

std::vector<std::string> ft_split(std::string str, const std::string &delimiter)
{
    std::vector<std::string> result;
    size_t index;
    while(str.size())
    {
        index = str.find_first_of(delimiter);
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

Server::Server() {
    this->_maxBodySize = 1000000;
}

Server::~Server() {
}

void	Server::setName(const std::string &name) {
    this->_names.push_back(name);
}

void	Server::setHost(const std::string &IP) {
    if (!checkIP(IP))
        throw ParserError("Invalid IP address", IP);
    else if (IP == "localhost")
        this->_host = "127.0.0.1";
    else
        this->_host = IP;
}

void	Server::setPort(const std::string &port) {
    if (!checkDigit(port))
        throw ParserError("Invalid port number", port);
    if (ft_stoi(port) > 65535)
        throw ParserError("Port number exceed", port);
    else
        this->_port = ft_stoi(port);
}

void	Server::setErrorPage(const std::string &code, const std::string &path) {
    if (!checkDigit(code))
        throw ParserError("Invalid error code", code);
    this->_errorPages.insert(std::pair<int, std::string>(ft_stoi(code), path));
}

void	Server::setMaxBodySize(const std::string &size) {
    int lastchar = size.length() - 1;
    if (checkDigit(size))
        this->_maxBodySize = ft_stoi(size);
    else if (checkDigit(size.substr(0, lastchar)))
    {
        if (size[lastchar] == 'k' || size[lastchar] == 'K')
            this->_maxBodySize = ft_stoi(size.substr(0, lastchar)) * 1000;
        else if (size[lastchar] == 'm' || size[lastchar] == 'M')
            this->_maxBodySize = ft_stoi(size.substr(0, lastchar)) * 1000000;
        else if (size[lastchar] == 'g' || size[lastchar] == 'G')
            this->_maxBodySize = ft_stoi(size.substr(0, lastchar)) * 1000000000;
        else
            throw ParserError("Invalid client buffet size", size);
    }
    else
        throw ParserError("Invalid client buffet size", size);
}

void    Server::setMimeType(const std::string &extension, const std::string &type) {
    if (extension[0] != '.')
        throw ParserError("Invalid extension", extension);
    else if (extension.length() == 1)
        throw ParserError("Invalid extension", extension);
    this->_mimeTypes.insert(std::pair<std::string, std::string>(extension, type));
}

void	Server::addLocation(Location location) {
    this->_routes.push_back(location);
}

void    Server::defaultErrorPages() {
    if (_errorPages.find(400) == _errorPages.end())
        _errorPages.insert(std::pair<int, std::string>(400, "error_pages/400.html"));
    if (_errorPages.find(403) == _errorPages.end())
        _errorPages.insert(std::pair<int, std::string>(403, "error_pages/403.html"));
    if (_errorPages.find(404) == _errorPages.end())
        _errorPages.insert(std::pair<int, std::string>(404, "error_pages/404.html"));
    if (_errorPages.find(405) == _errorPages.end())
        _errorPages.insert(std::pair<int, std::string>(405, "error_pages/405.html"));
    if (_errorPages.find(500) == _errorPages.end())
        _errorPages.insert(std::pair<int, std::string>(500, "error_pages/500.html"));
    if (_errorPages.find(505) == _errorPages.end())
        _errorPages.insert(std::pair<int, std::string>(505, "error_pages/501.html"));
}

const std::vector<std::string> &Server::getNames() const {
    return (this->_names);
}

const std::string &Server::getHost() const {
    return (this->_host);
}

size_t Server::getPort() const {
    return (this->_port);
}

std::string Server::getErrorPagePath(int code) const{
    if (this->_errorPages.count(code))
        return (this->_errorPages.find(code)->second);
    return std::string();
}

size_t Server::getMaxBodySize() const {
    return (this->_maxBodySize);
}

const std::vector<Location> &Server::getRoutes() const {
    return (this->_routes);
}

const MimeType &Server::getMimeTypes() const {
    return (this->_mimeTypes);
}

std::ostream& operator<<(std::ostream& os, const Server& server) {
    for (std::vector<std::string>::const_iterator it = server._names.begin(); it != server._names.end(); it++) {
        os << "Server name: " << *it << std::endl;
    }
    os << "Host: " <<  server.getHost() << std::endl;
    os << "Port: " << server.getPort() << std::endl;
    for (std::map<int, std::string>::const_iterator it = server._errorPages.begin(); it != server._errorPages.end(); it++) {
        os << "Error pages: " << std::endl;
        os << it->first << ": " << it->second << std::endl;
    }
    os << "Max body size: " << server.getMaxBodySize() << std::endl;
    for (std::vector<Location>::const_iterator it = server._routes.begin(); it != server._routes.end(); it++) {
        os << *it << std::endl;
    }
    os << "Mime types: " << std::endl;
    for (MimeType::const_iterator it = server._mimeTypes.begin(); it != server._mimeTypes.end(); it++) {
        os << it->first << ": " << it->second << std::endl;
    }
    return (os);
}

bool Server::checkDigit(const std::string &str) {
    for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
        if (!std::isdigit(*it))
            return (false);
    }
    return (true);
}

bool Server::checkIP(const std::string &IP)
{
    if (IP.compare("localhost") == 0)
        return (true);
        
    if (IP.length() < 7 || IP.length() > 15)
        return (false);
        
    std::vector<std::string> IPList = ft_split(IP, ".");
    if (IPList.size() != 4)
        return (false);
    
    for (std::vector<std::string>::const_iterator it = IPList.begin(); it != IPList.end(); it++)
    {
        if ((*it).length() > 3)
            return (false);
        else if (!checkDigit(*it))
            return (false);
        else if (ft_stoi(*it) < 0 || ft_stoi(*it) > 255)
            return (false);
    }
    return (true);
}
