/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:03:53 by chchin            #+#    #+#             */
/*   Updated: 2023/07/16 14:45:56 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <map>
# include <iostream>
# include <iterator>
# include "Location.hpp"
# include "MimeType.hpp"

typedef std::vector<std::string>::iterator conf_t;

std::vector<std::string> ft_split(std::string str, const std::string &delimiter);

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
		MimeType					_mimeTypes;

	public:
		void	setName(const std::string &name);
		void	setHost(const std::string &IP);
		void	setPort(const std::string &port);
		void	setErrorPage(const std::string &code, const std::string &path);
		void	setMaxBodySize(const std::string &size);
		void	addLocation(Location location);
		void	defaultErrorPages();
		void	setMimeType(const std::string &extension, const std::string &type);

		const std::vector<std::string>	&getNames() const;
		const std::string				&getHost() const;
		size_t							getPort() const;
		std::string						getErrorPagePath(int error_code) const;
		size_t							getMaxBodySize() const;
		const std::vector<Location>		&getRoutes() const;
		const MimeType					&getMimeTypes() const;

		bool checkDigit(const std::string &str);
		bool checkIP(const std::string &str);
		
	friend std::ostream& operator<<(std::ostream& os, const Server& server);
};


#endif
