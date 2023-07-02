/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/13 15:50:01 by chchin            #+#    #+#             */
/*   Updated: 2023/07/01 23:14:04 by chchin           ###   ########.fr       */
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
		throw ParserError("Invalid method in location", method);
	
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
        throw ParserError("Invalid autoindex in location", autoIndex);
}

void Location::setRedirection(std::string code, std::string url)
{
    if (code != "301" && code != "302" && code != "303" && code != "307" && code != "308")
        throw ParserError("Invalid redirection code in location", code);
    this->_redirection.first = ft_stoi(code);
    this->_redirection.second = url;
    this->_isRedirected = true;
}

void Location::setCgiExtension(std::string extension)
{
    if (!checkCgiExtension(extension))
        throw ParserError("Invalid Cgi extension", extension);
    this->_cgiExtension.push_back(extension);
}

void Location::setCgiPath(std::string extension, std::string path)
{
    if (!checkCgiExtension(extension))
        throw ParserError("Invalid Cgi extension", extension);
    this->_cgiPath[extension] = path;
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

bool Location::checkCgiExtension(std::string extension) const
{
    if (extension[0] != '.')
        return (false);
    return (true);
}

std::string Location::getCgiPath(std::string extension) const
{
    return (this->_cgiPath.find(extension)->second);
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
    out << "Cgi path:";
    for (std::map<std::string, std::string>::const_iterator it = location._cgiPath.begin(); it != location._cgiPath.end(); it++)
        out << it->first << " " << it->second << " ";
    out << std::endl;
    return (out);
}
