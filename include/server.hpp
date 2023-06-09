/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brook <brook@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:03:53 by chchin            #+#    #+#             */
/*   Updated: 2023/06/09 23:07:28 by brook            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <map>
# include <iostream>
# include "location.hpp"

class Location;

class Server {
	public:
		Server();
		~Server();

	private:
		std::vector<std::string>	_names;
		std::string					_IP;
		int							_port;
		std::map<int, std::string>	_errorPages;
		size_t						_clientBuffetSize;
		std::vector<Location>		_routes;

	public:
		void	setName(std::string name);
		void	setIP(std::string IP);
		void	setPort(std::string port);
		void	setErrorPage(int code, std::string path);
		void	setClientBuffetSize(size_t size);
		void	addLocation(Location location);
		void	defaultErrorPages();

		std::vector<std::string>	&getNames() const;
		std::string					&getIP() const;
		int							getPort() const;
		std::map<int, std::string>	&getErrorPages() const;
		size_t						getClientBuffetSize() const;
		std::vector<Location>		&getRoutes() const;
};

#endif
