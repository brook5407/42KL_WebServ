/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 15:50:01 by chchin            #+#    #+#             */
/*   Updated: 2023/06/25 23:50:01 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

int ft_stoi(std::string str)
{
    std::istringstream ss(str);
    int num;
    ss >> num;
    return num;
}

Location::Location()
{
    this->_isRedirected = false;
    this->_autoIndex = false;
}

Location::~Location()
{}

void Location::setPrefix(std::string prefix)
{
    this->_prefix = prefix;
}

void Location::setRoot(std::string path)
{
    this->_root = path;
}

void Location::setMethod(std::string method)
{
    if (method.compare("GET") && 
		method.compare("POST") && 
		method.compare("DELETE") &&
        method.compare("PUT"))
		throw std::invalid_argument("Error: Invalid method in location");
	
	_methods.insert(method);
}

void Location::setIndex(std::string path)
{
    this->_index.push_back(path);
}

void Location::setAutoIndex(std::string autoIndex)
{
    if (autoIndex == "on")
        this->_autoIndex = true;
    else if (autoIndex == "off")
        this->_autoIndex = false;
    else
        throw std::invalid_argument("Error: Invalid autoindex in location");
}

void Location::setRedirection(std::string code, std::string url)
{
    if (code != "301" && code != "302" && code != "303" && code != "307" && code != "308")
        throw std::invalid_argument("Error: Invalid redirection code in location");
    this->_redirection.first = ft_stoi(code);
    this->_redirection.second = url;
    this->_isRedirected = true;
}

void Location::setCgiExtension(std::string extension)
{
    this->_cgiExtension.push_back(extension);
}

std::string Location::getPrefix() const
{
    return (this->_prefix);
}

std::string Location::getRoot() const
{
    return (this->_root);
}

std::set<std::string> &Location::getMethods()
{
    return (this->_methods);
}

std::vector<std::string> &Location::getIndex()
{
    return (this->_index);
}

bool Location::checkAutoIndex() const
{
    return (this->_autoIndex);
}

std::pair<int, std::string> &Location::getRedirection()
{
    return (this->_redirection);
}

bool Location::checkRedirection() const
{
    return (this->_isRedirected);
}

std::vector<std::string> &Location::checkCgiExtension()
{
    return (this->_cgiExtension);
}

std::ostream &operator<<(std::ostream &out, const Location &location)
{
    out << "Prefix: " << location._prefix << std::endl;
    out << "Root: " << location._root << std::endl;
    out << "Methods: ";
    for (std::set<std::string>::iterator it = location._methods.begin(); it != location._methods.end(); it++)
        out << *it << " ";
    out << std::endl;
    out << "Index page: ";
    for (std::vector<std::string>::const_iterator it = location._index.begin(); it != location._index.end(); it++)
        out << *it << " ";
    out << std::endl;
    out << "Autoindex: " << location._autoIndex << std::endl;
    out << "Redirection: " << location._redirection.first << " " << location._redirection.second << std::endl;
    out << "Cgi extension: ";
    for (std::vector<std::string>::const_iterator it = location._cgiExtension.begin(); it != location._cgiExtension.end(); it++)
        out << *it << " ";
    out << std::endl;
    return (out);
}
