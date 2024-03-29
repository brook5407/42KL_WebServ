/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 23:41:40 by chchin            #+#    #+#             */
/*   Updated: 2023/07/16 14:49:13 by chchin           ###   ########.fr       */
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
        
        void    parseConfig(std::vector<std::string> &config);
        void    parseServer(conf_t &line_pos, const conf_t &end);
        Location parseLocation(conf_t &line_pos, const conf_t &end);

        void    checkServer();
        bool    checkLastNonSpaceCharacter(const std::string &str);

        std::vector<Server>   &getServers();
        
    private:
        std::vector<Server>   _servers;

};

#endif