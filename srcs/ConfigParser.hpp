/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kishyan <kishyan@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 23:41:40 by chchin            #+#    #+#             */
/*   Updated: 2023/06/25 23:27:15 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "Server.hpp"

typedef std::vector<std::string>::iterator conf_t;

class ConfigParser
{
    public:
        ConfigParser(std::string configFile);
        ~ConfigParser();
        
        void    parseConfig(std::vector<std::string> config);
        void    parseServer(conf_t &line_pos, conf_t end);
        Location parseLocation(conf_t &line_pos, conf_t end);

        void    checkServer();

        std::vector<Server>   &getServers();
        
    private:
        std::vector<Server>   _servers;
};

#endif