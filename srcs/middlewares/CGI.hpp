#ifndef CGI_HPP
# define CGI_HPP

# include "Response.hpp"
# include <string>
# include <vector>
# include <exception>

class CGI
{
	public:
		CGI(Response);
		// ~CGI(void);
		void	setup_bash(const std::string &handler, const std::string &script, const std::string &body);
		void	response(void);
		bool	is_timeout(int timeout);
		void	add_envp(std::string key, const std::string &value);
		void	set_session_id(const std::string &id);
		void	add_local_envp(const char *var);

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
		// bool	check_file(std::string &route);
		// void	_execute_cgi(const std::string &body);
		static char	**string_to_char(const std::vector<std::string> &vec);

		time_t						_start_time;
		// FILE						*file_in;
		// FILE						*file_out;
		// int						file_in_fd;
		int							_file_out_fd;
		std::vector<std::string>	_envp;
		std::string					_session_id;

};

#endif