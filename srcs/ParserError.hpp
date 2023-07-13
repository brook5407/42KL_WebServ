/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserError.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 23:20:46 by chchin            #+#    #+#             */
/*   Updated: 2023/07/01 23:20:54 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSERERROR_HPP
# define PARSERERROR_HPP

#include <exception>
#include <string>

class ParserError: public std::exception
{
    private:
        std::string _msg;
    public:
        ParserError();
        ParserError(const std::string &error, const std::string &line);
        ~ParserError() throw();
        const char *what () const throw ();
};

#endif