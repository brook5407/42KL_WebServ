/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 15:50:21 by chchin            #+#    #+#             */
/*   Updated: 2023/06/25 23:42:45 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

std::vector<std::string> ft_split(std::string str, std::string delimiter)
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

Server::Server() {
}

Server::~Server() {
}

void	Server::setName(std::string name) {
    this->_names.push_back(name);
}

void	Server::setHost(std::string IP) {
    if (!checkIP(IP))
        throw std::invalid_argument("Error: Invalid IP address");
    else if (IP == "localhost")
        this->_host = "127.0.0.1";
    else
        this->_host = IP;
}

void	Server::setPort(std::string port) {
    if (!checkDigit(port))
        throw std::invalid_argument("Error: Invalid port number");
    if (ft_stoi(port) > 65535)
        throw std::invalid_argument("Error Invalid port number");
    else
        this->_port = ft_stoi(port);
}

void	Server::setErrorPage(std::string code, std::string path) {
    if (!checkDigit(code))
        throw std::invalid_argument("Error: Invalid error code");
    this->_errorPages.insert(std::pair<int, std::string>(ft_stoi(code), path));
}

void	Server::setMaxBodySize(std::string size) {
    int lastchar = size.length() - 1;
    if (size[lastchar] != 'm')
        throw std::invalid_argument("Error: Invalid client buffet size");
    size.erase(lastchar);
    if (!checkDigit(size))
            throw std::invalid_argument("Error: Invalid client buffet size");
    this->_maxBodySize = ft_stoi(size);
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

std::vector<std::string> &Server::getNames(){
    return (this->_names);
}

std::string Server::getHost() const {
    return (this->_host);
}

size_t Server::getPort() const {
    return (this->_port);
}

std::string Server::getErrorPagePath(int code) const{
    return (this->_errorPages.find(code)->second);
}

size_t Server::getMaxBodySize() const {
    return (this->_maxBodySize);
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
    return (os);
}

bool Server::checkDigit(std::string str) {
    for (std::string::iterator it = str.begin(); it != str.end(); it++) {
        if (!std::isdigit(*it))
            return (false);
    }
    return (true);
}

bool Server::checkIP(std::string IP)
{
    if (IP.compare("localhost") == 0)
        return (true);
        
    if (IP.length() < 7 || IP.length() > 15)
        return (false);
        
    std::vector<std::string> IPList = ft_split(IP, ".");
    if (IPList.size() != 4)
        return (false);
    
    for (std::vector<std::string>::iterator it = IPList.begin(); it != IPList.end(); it++)
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
