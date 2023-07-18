#include "CGI.hpp"
# include <cstdio>

CGI::CGI(Response response)
: child_pid(-1), _response(response), _start_time(time(NULL)), _file_out_fd(-1)
{
	_response._connection.status() = WAITING;
}

// CGI::~CGI()
// {
// 	if (file_in)
// 		fclose(file_in);
// }

void	CGI::setup_bash(const std::string &handler, const std::string &script, const std::string &body)
{
	std::vector<std::string>	argv;

	argv.push_back(handler);
	argv.push_back(script);

	FILE *file_in = tmpfile();
	int file_in_fd = fileno(file_in);
	int check = write(file_in_fd, body.c_str(), body.size());
	if (check == -1)
		throw CGIException();
	lseek(file_in_fd, 0, SEEK_SET);

	if (dup2(_file_out_fd, STDOUT_FILENO) == -1)
		throw CGIException();
	if (dup2(file_in_fd, STDIN_FILENO) == -1)
		throw CGIException();

	//unistd.h sysconf(_SC_OPEN_MAX)
	for (int i = 3; i < 1024; ++i)
		close(i);
	char **arguments = string_to_char(argv);
	char **envp = string_to_char(this->_envp);
	execve(*arguments, arguments, envp);
	perror("execve");
	delete [] arguments;
	delete [] envp;
	fclose(file_in);
	exit(1);
}

bool	CGI::is_timeout(int timeout)
{
	double timediff = difftime(time(NULL), _start_time);
	return (timediff >= timeout);
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

void	CGI::set_session_id(const std::string &id)
{
	_session_id = id;
	FILE	*file_out = tmpfile();
	_file_out_fd = fileno(file_out);
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
