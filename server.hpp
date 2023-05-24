/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:03:53 by chchin            #+#    #+#             */
/*   Updated: 2023/05/23 12:50:05 by chchin           ###   ########.fr       */
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
		Server(Server const &src);
		~Server();

	private:
		std::vector<std::string> _names;
		std::string	_IP;
		int			_port;
		std::map<int, std::string>	_errorPages;
		std::size_t					_clientBuffetSize;
		std::vector<Location>		_routes;
}
