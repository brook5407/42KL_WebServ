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
		void	setup_bash(const std::string &handler, const std::string &script, const std::string &body);
		void	response(void);
		int		is_timeout(int timeout);
		void	add_envp(std::string key, const std::string &value);
		void	set_session_id(const std::string &id);

		class CGIException : public std::exception
		{
			public:
				const char *what() const throw()
				{
					return ("CGI ERROR!");
				}
		};

		pid_t						child_pid;
		Response					_response;
	private:
		bool	check_file(std::string &route);
		void	_execute_cgi(const std::string &body);
		char	**string_to_char(const std::vector<std::string> &vec);

		std::vector<std::string>	argv;
		time_t						start_time;
		FILE						*file_in;
		FILE						*file_out;
		int							file_in_fd;
		int							file_out_fd;
		std::vector<std::string>	_envp;
		std::string					_session_id;

};

#endif