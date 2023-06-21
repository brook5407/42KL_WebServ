#include "CGI.hpp"
#include <stdio.h>

CGI::CGI(Response response) : _response(response),  child_pid(-1), file_in(NULL), file_out(NULL),
 file_in_fd(-1), file_out_fd(-1)
{ }

CGI::~CGI()
{
	if (file_in)
		fclose(file_in);
	if (file_out)
		fclose(file_out);
}

bool	CGI::check_file(std::string &route)
{
	std::ifstream	file(route.c_str());
	return file.good();
}

void	CGI::setup_bash(std::string script)
{
	argv.push_back("/bin/bash");
	argv.push_back(script);
	_execute_cgi();
}

void	CGI::_execute_cgi(void)
{
	//hardcoded value for body
	_body = "hey world";

	// if (pipe(pipefd) == -1)
	// 	throw CGIException();

	// check if file exists
	if (!check_file(argv[1]))
	{
		throw CGIException();
		std::string	fav_req = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 9\nConnection: close\n\nNot Found";
		int sent = send(_socket, fav_req.c_str(), fav_req.size(), 0);
		if (sent < 0)
			throw std::runtime_error("Send error");
		close(_socket);
		return;
	}

	// create in out files
	file_in = tmpfile();
	file_out = tmpfile();

	// get fd from files
	file_in_fd = fileno(file_in);
	file_out_fd = fileno(file_out);

	// write body to stdin
	write(file_in_fd, _body.c_str(), _body.size());
	lseek(file_in_fd, 0, SEEK_SET);

	//start time for timeout
	start_time = time(NULL);

	child_pid = fork();
	if (child_pid == -1)
		throw CGIException();
	else if (child_pid == 0)
	{
		// create argument for execve
		//char * const * arg = NULL;
		char* const arguments[] = {const_cast<char*>(argv[0].c_str()), const_cast<char*>(argv[1].c_str()), NULL};

		if (dup2(file_out_fd, STDOUT_FILENO) == -1)
			throw CGIException();

		if (dup2(file_in_fd, STDIN_FILENO) == -1)
			throw CGIException();

		execve(argv[0].c_str(), arguments, NULL);
		std::cout << "execve failed" << std::endl;
		exit(1);
	}
}


int		CGI::is_timeout(int timeout)
{
	// if want to use WNOHANG, have to add a slight delay
	// allows child process to finish first before trying to read from fd ?
	//usleep(1000000);
	if (difftime(time(NULL), start_time) > timeout)
	{
		kill(child_pid, SIGKILL); // test SIGCHLD trigger
		return true;
	}
	// if (waitpid(child_pid, NULL, WNOHANG) < 0)
	// 	return false;

	// while (waitpid(child_pid, NULL, WNOHANG) == 0)
	// {
	// 	if (difftime(time(NULL), start_time) > timeout)
	// 	{
	// 		kill(child_pid, SIGKILL);
	// 		return true ;
	// 	}
	// }
	return false;
}

const std::string &CGI::get_output(void)
{
	char cgiBuffer[30000];
	int bytes = 1;

	if (lseek(file_out_fd, 0, SEEK_SET) == -1)
	{
		perror("lseek");
	}
	else
	{
		while (bytes > 0)
		{
			bytes = read(file_out_fd, cgiBuffer, sizeof(cgiBuffer));
			output += std::string(cgiBuffer, bytes);
		}
	}
	std::cout << "output size: " << output.size() << std::endl;
	// waitpid(child_pid, NULL, 0);
	// lseek(file_out_fd, 0, SEEK_SET);
	// int bytes = read(file_out_fd, cgiBuffer, 10000);
	// std::cout << cgiBuffer << std::endl;
	// output += std::string(cgiBuffer, bytes);
	return output;
}

void	CGI::response(void)
{
	_response.write(get_output());
	_response.end();
	// int sent = send(_socket, output.c_str(), output.size(), 0);

	// if (sent < 0)
	// 	throw CGIException();
	// close(_socket);
}