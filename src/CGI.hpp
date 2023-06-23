#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>
# include <fstream>
# include <stdlib.h>
# include <stdio.h>
# include <cstring>
# include <string>
# include <vector>
# include <map>
# include <sys/socket.h>
# include <sys/wait.h>
# include <netinet/in.h>
# include <exception>
# include <fcntl.h>
# include <unistd.h>
# include <errno.h>
# include <iterator>
# include "Response.hpp"

class CGI
{
	public:
		CGI(Response);
		~CGI(void);
		bool	check_file(std::string &route);
		void	response(void);
		void	setup_bash(std::string script);
		int		is_timeout(int timeout);


		class CGIException : public std::exception
		{
			public:
				const char *what() const throw()
				{
					return ("CGI ERROR!");
				}
		};
		std::string					output;
		int							_socket;
		Response					_response;
		pid_t						child_pid;
	private:
		void	_execute_cgi(void);
		const std::string &get_output(void);
		std::vector<std::string>	argv;
		time_t						start_time;
		std::string					_body;
		FILE						*file_in;
		FILE						*file_out;
		int							file_in_fd;
		int							file_out_fd;
};

#endif