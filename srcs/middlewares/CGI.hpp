#ifndef CGI_HPP
# define CGI_HPP

# include "Response.hpp"
# include <string>
# include <vector>
# include <exception>

class CGI
{
	public:
		CGI(Response &response);
		void	exec(const std::string &argv0, const std::string &argv1, const std::string &body);
		void	set_pid(pid_t pid);
		void	set_session_id(const std::string &session_id);
		pid_t	get_pid(void) const;
		void	response(void);
		bool	timeout(std::size_t execution_timeout_sec);
		void	add_envp(std::string key, const std::string &value);
		void	add_local_envp(const char *var);

		class CGIException : public std::exception
		{
			public:
				const char *what() const throw()
				{
					return ("CGI ERROR!");
				}
		};


	private:
		pid_t						_pid;
		Response					_response;
		const time_t				_start_time;
		int							_file_out_fd;
		std::vector<std::string>	_envp;
		std::string					_session_id;

		CGI(void);
		static char		**string_to_char(const std::vector<std::string> &vec);
};

#endif
