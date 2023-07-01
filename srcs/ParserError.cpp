/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserError.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chchin <chchin@student.42kl.edu.my>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/01 23:18:54 by chchin            #+#    #+#             */
/*   Updated: 2023/07/01 23:21:34 by chchin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParserError.hpp"

ParserError::ParserError() {}

ParserError::ParserError(std::string error, std::string line){
    _msg = "\033[1m\033[31mError: \033[0m" + error + " => " + line.substr(line.find_first_not_of(" \t"));
}

ParserError::~ParserError() throw() {}

const char *ParserError::what () const throw ()
{
    return _msg.c_str();
}