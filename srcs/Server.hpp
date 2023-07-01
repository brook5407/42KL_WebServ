/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:03:53 by chchin            #+#    #+#             */
/*   Updated: 2023/07/01 23:16:17 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <map>
# include <iostream>
# include <iterator>
# include "Location.hpp"

typedef std::vector<std::string>::iterator conf_t;

std::vector<std::string> ft_split(std::string str, std::string delimiter);

class Location;

class Server {
	public:
		Server();
		~Server();

	private:
		std::vector<std::string>	_names;
		std::string					_host;
		int							_port;
		std::map<int, std::string>	_errorPages;
		size_t						_maxBodySize;
		std::vector<Location>		_routes;

	public:
		void	setName(std::string name);
		void	setHost(std::string IP);
		void	setPort(std::string port);
		void	setErrorPage(std::string code, std::string path);
		void	setMaxBodySize(std::string size);
		void	addLocation(Location location);
		void	defaultErrorPages();

		std::vector<std::string>	&getNames();
		std::string					getHost() const;
		size_t						getPort() const;
		std::string					getErrorPagePath(int error_code) const;
		size_t						getMaxBodySize() const;
		std::vector<Location>		&getRoutes();

		bool checkDigit(std::string str);
		bool checkIP(std::string str);
		std::vector<std::string> split(std::string str, std::string delimiter);
		
		friend class ParserError;
		
	friend std::ostream& operator<<(std::ostream& os, const Server& server);
};


#endif
