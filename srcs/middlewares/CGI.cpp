#include "CGI.hpp"
#include <cstdlib>
#include <csignal>

CGI::CGI(Response &response)
: _pid(-1), _response(response), _start_time(time(NULL))
{
	FILE	*file_out = tmpfile();
	_file_out_fd = fileno(file_out);
}

void	CGI::set_pid(pid_t pid)
{
	_pid = pid;
}

void	CGI::set_session_id(const std::string &session_id)
{
	_session_id = session_id;
}

pid_t	CGI::get_pid(void) const
{
	return (_pid);
}

void	CGI::exec(const std::string &argv0, const std::string &argv1, const std::string &body)
{
	FILE *file_in = tmpfile();
	int file_in_fd = fileno(file_in);
	if (write(file_in_fd, body.c_str(), body.size()) != static_cast<ssize_t>(body.size()))
		exit(1);
	lseek(file_in_fd, 0, SEEK_SET);
	if (dup2(_file_out_fd, STDOUT_FILENO) == -1)
		exit(1);
	if (dup2(file_in_fd, STDIN_FILENO) == -1)
		exit(1);

	for (int i = 3; i < OPEN_MAX; ++i)
		close(i);
	char	*argv[] = 
	{
		const_cast<char*>(argv0.c_str()),
		const_cast<char*>(argv1.c_str()),
		NULL
	};
	char **envp = string_to_char(this->_envp);
	execve(*argv, argv, envp);
	perror("execve");
	delete [] envp;
	fclose(file_in);
	exit(1);
}

bool	CGI::timeout(std::size_t execution_timeout_sec)
{
	double timediff = difftime(time(NULL), _start_time);
	bool is_timeout = (timediff >= execution_timeout_sec);
	if (is_timeout)
	{
		_response.send_content(502, "Process has timed out");
		kill(_pid, SIGKILL);
	}
	return (is_timeout);
}

void	CGI::response(void)
{
	_response.send_cgi_fd(_file_out_fd, _session_id);
}

char	**CGI::string_to_char(const std::vector<std::string> &vec)
{
	char **array = new char*[vec.size() + 1];

	for (size_t i = 0; i < vec.size(); ++i)
		array[i] = const_cast<char*>(vec[i].c_str());
	array[vec.size()] = NULL;
	return (array);
}

void	CGI::add_envp(std::string key, const std::string &value)
{
	for (size_t i = 0; i < key.size(); ++i)
	{
		if (key[i] == '-')
			key[i] = '_';
		else
			key[i] = std::toupper(key[i]);
	}
	_envp.push_back(key + "=" + value);
}

void	CGI::add_local_envp(const char *var)
{
	_envp.push_back(std::string(var));
}
