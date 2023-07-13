/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:04:51 by chchin            #+#    #+#             */
/*   Updated: 2023/07/04 10:20:15 by chchin           ###   ########.fr       */
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

int ft_stoi(std::string str);

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
		void	setPrefix(std::string prefix);
		void	setRoot(std::string path);
		void	setMethod(std::string method);
		void	setIndex(std::string path);
		void 	setAutoIndex(std::string autoIndex);
		void	setRedirection(std::string code, std::string url);
		void	setCgiExtension(std::string extension);
		void	setCgiPath(std::string extension, std::string path);
		void	setMaxBodySize(std::string size);

		std::string 			getPrefix() const;
		std::string 			getRoot() const;
		std::set<std::string>	&getMethods();
		std::vector<std::string>&getIndex();
		bool					checkAutoIndex() const;
		std::pair<int, std::string> &getRedirection();
		bool 					checkRedirection() const;
		bool					checkCgiExtension(std::string extension) const;
		std::string				getCgiPath(std::string extension) const;
		size_t					getMaxBodySize() const;

	friend std::ostream &operator<<(std::ostream &out, const Location &location);
};

#endif