/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brook <brook@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:04:53 by chchin            #+#    #+#             */
/*   Updated: 2023/06/09 13:35:44 by brook            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

Location::Location()
{
    this->_isRedirected = false;
    this->_autoIndex = false;
}

Location::~Location()
{
}

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
		method.compare("DELETE"))
		throw std::invalid_argument("Invalid method in location");
	
	_allowedMethods.insert(method);
}

void Location::setIndexPage(std::string path)
{
    this->_indexPage = path;
}

void Location::setAutoIndex(std::string on_off)
{
    if (on_off == "on")
        this->_autoIndex = true;
    else if (on_off == "off")
        this->_autoIndex = false;
    else
        throw std::invalid_argument("Invalid autoindex in location");
}

void Location::setRedirection(int code, std::string url)
{
    this->_redirection.first = code;
    this->_redirection.second = url;
    this->_isRedirected = true;
}

std::getPrefix &Location::getPrefix() const
{
    return (this->_prefix);
}

std::getRoot &Location::getRoot() const
{
    return (this->_root);
}

std::set<std::string> &Location::getMethods() const
{
    return (this->_allowedMethods);
}

std::string &Location::getIndexPage() const
{
    return (this->_indexPage);
}

bool Location::checkAutoIndex() const
{
    return (this->_autoIndex);
}

std::pair<int, std::string> &Location::getRedirection() const
{
    return (this->_redirection);
}

bool Location::checkRedirection() const
{
    return (this->_isRedirected);
}