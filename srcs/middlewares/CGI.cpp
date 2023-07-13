#include "CGI.hpp"

CGI::CGI(Response response)
: child_pid(-1), _response(response),
file_in(NULL), file_out(NULL), file_in_fd(-1), file_out_fd(-1)
{
	_response._connection.status() = WAITING;
}

CGI::~CGI()
{
	if (file_in)
		fclose(file_in);
}

bool	CGI::check_file(std::string &route)
{
	std::ifstream	file(route.c_str());
	return file.good();
}

void	CGI::setup_bash(const std::string &handler, const std::string &script, const std::string &body)
{
	if (handler.size())
		argv.push_back(handler); // "/bin/bash"
	argv.push_back(script);
	_execute_cgi(body);
}

void	CGI::_execute_cgi(const std::string &body)
{
	if (!check_file(argv[0]))
		throw CGIException();

	file_out = tmpfile();
	file_out_fd = fileno(file_out);

	start_time = time(NULL);

	child_pid = fork();

	if (child_pid == -1)
		throw CGIException();
	else if (child_pid == 0)
	{
		// if (body.empty() == true)
		{
			file_in = tmpfile();
			file_in_fd = fileno(file_in);
			write(file_in_fd, body.c_str(), body.size());
			lseek(file_in_fd, 0, SEEK_SET);
			if (dup2(file_in_fd, STDIN_FILENO) == -1)
				throw CGIException();
		}

		if (dup2(file_out_fd, STDOUT_FILENO) == -1)
			throw CGIException();

		char **arguments = string_to_char(this->argv);
		char **envp = string_to_char(this->_envp);
		execve(*arguments, arguments, envp);
		perror("execve");
		delete [] arguments;
		delete [] envp;
		// std::cout << "execve failed" << std::endl;
		exit(1);
	}
}

int		CGI::is_timeout(int timeout)
{
	double timediff = difftime(time(NULL), start_time);
	if (timediff >= timeout)
		return true;
	return false;
}

void	CGI::response(void)
{
	_response.send_cgi_fd(file_out_fd, _session_id);
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