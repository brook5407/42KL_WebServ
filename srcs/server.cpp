/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brook <brook@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:03:51 by chchin            #+#    #+#             */
/*   Updated: 2023/06/09 23:07:02 by brook            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

bool checkIP(std::string IP) {
    if (IP.compare("localhost") == 0)
        return (true);
        
    if (IP.length() < 7 || IP.length() > 15)
        return (false);
        
    std::vector<std::string> IPList = ft_split(IP, '.');
    if (IPList.size() != 4)
        return (false);
    
    for (std::vector<std::string>::iterator it = IPList.begin(); it != IPList.end(); it++) {
        if ((*it).length() > 3)
            return (false);
        for (std::string::iterator it2 = (*it).begin(); it2 != (*it).end(); it2++) {
            if (!std::isdigit(*it2))
                return (false);
        }
        if (std::stoi(*it) < 0 || std::stoi(*it) > 255)
            return (false);
    }
    return (true);
}

Server::Server() {
}

Server::~Server() {
}

void	Server::setName(std::string name) {
    this->_names.push_back(name);
}

void	Server::setIP(std::string IP) {
    if (!checkIP(IP))
        throw std::invalid_argument("Invalid IP address");
    else
        this->_IP = IP;
}

void	Server::setPort(std::string port) {
    if (std::string:;iterator it = port.begin(); it != port.end(); it++) {
        if (!std::isdigit(*it))
            throw std::invalid_argument("Invalid port number");
    }
    if (std::stoi(port) > 65535)
        throw std::invalid_argument("Invalid port number");
    else
        this->_port = port;
}

void	Server::setErrorPage(int code, std::string path) {
    this->_errorPages.insert(std::pair<int, std::string>(code, path));
}

void	Server::setClientBuffetSize(size_t size) {
    this->_clientBuffetSize = size;
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

