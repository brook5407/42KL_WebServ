/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:04:51 by chchin            #+#    #+#             */
/*   Updated: 2023/07/16 14:41:07 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <sstream>
# include <set>
# include <vector>
# include <map>
# include "ParserError.hpp"

int ft_stoi(const std::string &str);

class Location {
	public:
		Location();
		~Location();

	private:
		std::string							_prefix;
		std::string 						_root;
		std::set<std::string>				_methods;
		std::vector<std::string>			_index;
		bool								_autoIndex;
		std::pair<int, std::string>			_redirection;
		bool								_isRedirected;
		size_t								_maxBodySize;
		std::vector<std::string>			_cgiExtension;
		std::map<std::string, std::string>	_cgiPath;

	public:
		void	setPrefix(const std::string &prefix);
		void	setRoot(const std::string &path);
		void	setMethod(const std::string &method);
		void	setIndex(const std::string &path);
		void 	setAutoIndex(const std::string &autoIndex);
		void	setRedirection(const std::string &code, const std::string &url);
		void	setCgiExtension(const std::string &extension);
		void	setCgiPath(const std::string &extension, const std::string &path);
		void	setMaxBodySize(const std::string &size);

		const std::string 					&getPrefix() const;
		const std::string 					&getRoot() const;
		const std::set<std::string>			&getMethods() const;
		const std::vector<std::string>		&getIndex() const;
		bool								checkAutoIndex() const;
		const std::pair<int, std::string>	&getRedirection() const;
		bool 								checkRedirection() const;
		bool								checkCgiExtension(const std::string &extension) const;
		std::string							getCgiPath(const std::string &extension) const;
		size_t								getMaxBodySize() const;

	friend std::ostream &operator<<(std::ostream &out, const Location &location);
};

#endif