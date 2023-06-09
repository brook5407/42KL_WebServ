/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brook <brook@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 10:04:51 by chchin            #+#    #+#             */
/*   Updated: 2023/06/09 01:43:05 by brook            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
		std::string					_indexPage;
		bool						_autoIndex;
		std::pair<int, std::string>	_redirection;
		bool						_isRedirected;

	public:
		void	setPrefix(std::string prefix);
		void	setRoot(std::string path);
		void	setMethod(std::string method);
		void	setIndexPage(std::string path);
		void	setAutoIndex(std::string on_off);
		void	setRedirection(int code, std::string url);

		std::string &getPrefix() const;
		std::string &getRoot() const;
		std::set<std::string> &getMethods() const;
		std::string &getIndexPage() const;
		bool checkAutoIndex() const;
		std::pair<int, std::string> &getRedirection() const;
		bool checkRedirection() const;
};

#endif