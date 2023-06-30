#include "CGI.hpp"
// #include <stdio.h>

CGI::CGI(Response response)
: child_pid(-1), _response(response),
file_in(NULL), file_out(NULL), file_in_fd(-1), file_out_fd(-1)
{ }

CGI::~CGI()
{
	if (file_in)
		fclose(file_in);
	// if (file_out)
		// fclose(file_out);
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
	// if (pipe(pipefd) == -1)
	// 	throw CGIException();

	// check if file exists
	if (!check_file(argv[0]))
	{
		throw CGIException();
		// std::string	fav_req = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 9\nConnection: close\n\nNot Found";
		// int sent = send(_socket, fav_req.c_str(), fav_req.size(), 0);
		// if (sent < 0)
		// 	throw std::runtime_error("Send error");
		// close(_socket);
		// return;
	}

	// create in out files
	file_out = tmpfile();

	// get fd from files
	file_out_fd = fileno(file_out);

	//start time for timeout
	start_time = time(NULL);

	child_pid = fork();
	if (child_pid == -1)
		throw CGIException();
	else if (child_pid == 0)
	{
		// write body to stdin
		file_in = tmpfile();
		file_in_fd = fileno(file_in);
		write(file_in_fd, body.c_str(), body.size());
		lseek(file_in_fd, 0, SEEK_SET);

		// create argument for execve
		//char * const * arg = NULL;
		// char* const arguments[] = {const_cast<char*>(argv[0].c_str()), const_cast<char*>(argv[1].c_str()), NULL};
		// char* const arguments[] = {const_cast<char*>(argv[1].c_str()), NULL};
		// char* envp[this->_envp.size()] = {};
		// for (size_t i = 0; i < this->_envp.size(); i++)
		// 	envp[i] = const_cast<char*>(this->_envp[i].c_str());

		if (dup2(file_out_fd, STDOUT_FILENO) == -1)
			throw CGIException();

		if (dup2(file_in_fd, STDIN_FILENO) == -1)
			throw CGIException();

		char **arguments = convert_envp(this->argv);
		char **envp = convert_envp(this->_envp);
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
	//std::cout << timediff << std::endl;
	if (timediff >= timeout)
	{
		// kill(child_pid, SIGKILL); // test SIGCHLD trigger
		return true;
	}
	return false;
}

// const std::string &CGI::get_output(void)
// {
// 	char cgiBuffer[30000];
// 	int bytes = 1;

// 	if (lseek(file_out_fd, 0, SEEK_SET) == -1)
// 	{
// 		perror("lseek");
// 	}
// 	else
// 	{
// 		while (bytes > 0)
// 		{
// 			bytes = read(file_out_fd, cgiBuffer, sizeof(cgiBuffer));
// 			output += std::string(cgiBuffer, bytes);
// 		}
// 	}
// 	std::cout << "output size: " << output.size() << std::endl;
// 	//todo create header here rather than in file
// 	// output = create_header();
// 	// waitpid(child_pid, NULL, 0);
// 	// lseek(file_out_fd, 0, SEEK_SET);
// 	// int bytes = read(file_out_fd, cgiBuffer, 10000);
// 	// std::cout << cgiBuffer << std::endl;
// 	// output += std::string(cgiBuffer, bytes);
// 	return output;
// }

void	CGI::response(void)
{
	_response.send_cgi_fd(file_out_fd);
	// _response.send_content(get_output());
	// _response.end();
}

char	**CGI::convert_envp(const std::vector<std::string> &vec)
{
	char **array = new char*[vec.size() + 1];

    for (size_t i = 0; i < vec.size(); ++i)
		array[i] = const_cast<char*>(vec[i].c_str());
	array[vec.size()] = NULL;
	return (array);
}
