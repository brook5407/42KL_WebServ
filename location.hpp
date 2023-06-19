/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:04:51 by chchin            #+#    #+#             */
/*   Updated: 2023/06/19 00:42:42 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <set>

class Location {
	public:
		Location();
		~Location();

	private:
		std::string					_prefix;
		std::string 				_root;
		std::set<std::string>		_methods;
		std::string					_index;
		bool						_autoIndex;
		std::pair<int, std::string>	_redirection;
		bool						_isRedirected;
		bool						_cgiExtension;

	public:
		void	setPrefix(std::string prefix);
		void	setRoot(std::string path);
		void	setMethod(std::string method);
		void	setIndex(std::string path);
		void	setAutoIndex(std::string on_off);
		void	setRedirection(std::string code, std::string url);
		void	setCgiExtension(std::string extension);

		std::string 			getPrefix() const;
		std::string 			getRoot() const;
		std::set<std::string>	&getMethods();
		std::string 			getIndex() const;
		bool					checkAutoIndex() const;
		std::pair<int, std::string> &getRedirection();
		bool 					checkRedirection() const;
		bool 					checkCgiExtension() const;

	friend std::ostream &operator<<(std::ostream &out, const Location &location);
};

#endif