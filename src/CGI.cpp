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
	// for (size_t i = 0; i < envp.size(); i++)
	// 	delete[] _c_envp[i];
	// delete[] _c_envp;
}

bool	CGI::check_file(std::string &route)
{
	std::ifstream	file(route.c_str());
	return file.good();
}

void	CGI::setup_bash(std::string script)
{
	std::string extension = script.substr(script.find_last_of('.'));
	if (extension == ".sh")
		argv.push_back("/bin/bash");
	else if (extension == ".py")
		argv.push_back("/usr/bin/python3");
	else if (extension == ".php")
		argv.push_back("/usr/bin/php");
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
	// if (!check_file(argv[1]))
	// {
	// 	throw CGIException();
	// 	std::string	fav_req = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 9\nConnection: close\n\nNot Found";
	// 	int sent = send(_socket, fav_req.c_str(), fav_req.size(), 0);
	// 	if (sent < 0)
	// 		throw std::runtime_error("Send error");
	// 	close(_socket);
	// 	_response.status(404);
	// 	_response.write("File not found!");
	// 	return;
	// }

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
		char **_c_envp = convert_envp();
		execve(argv[0].c_str(), arguments, _c_envp);
		for (size_t i = 0; i < envp.size(); i++)
			delete[] _c_envp[i];
		delete[] _c_envp;
		std::cout << "execve failed" << std::endl;
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
	//todo create header here rather than in file
	output = create_header();
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

void	CGI::add_envp(const char *var)
{
	this->envp.push_back(var);
}

char	**CGI::convert_envp(void)
{
	char **_c_envp = new char*[envp.size()];

    for (size_t i = 0; i < envp.size(); ++i)
    {
        _c_envp[i] = new char[envp[i].size() + 1];
        std::strcpy(_c_envp[i], envp[i].c_str());
    }
	return (_c_envp);
}

void	CGI::add_CGI_var(Request &request)
{
	std::string	var;

	var = "REDIRECT_STATUS=200";
	add_envp(var.c_str());
	size_t size = request._content_length;
	std::stringstream ss;
	ss << size;
	var = "CONTENT_LENGTH=" +  ss.str();
	add_envp(var.c_str());
	var = "CONTENT_TYPE=" + request._headers["Content-Type"];
	add_envp(var.c_str());
	var = "GATEWAY_INTERFACE=CGI/1.1";
	add_envp(var.c_str());
	var = "PATH_INFO=" + (request._route->find("root"))->second + request._uri;
	add_envp(var.c_str());
	var = "PATH_TRANSLATED=" + (request._route->find("root"))->second + request._uri;
	add_envp(var.c_str());
	var = "REQUESTED_URI=" + (request._route->find("root"))->second + request._uri;
	add_envp(var.c_str());
	var = "REQUEST_METHOD=" + request._method;
	add_envp(var.c_str());
	var = "REMOTE_ADDR" + request._headers["Host"];
	add_envp(var.c_str());
	var = "SCRIPT_NAME=" + request._script_name;
	add_envp(var.c_str());
	var = "SERVER_PROTOCOL=HTTP/1.1";
	add_envp(var.c_str());
	var = "SERVER_SOFTWARE=webserv";
	add_envp(var.c_str());
	size_t pos = request._headers["Host"].find(':');
	var = "SERVER_PORT=" + request._headers["Host"].substr(pos + 1);
	add_envp(var.c_str());
	//todo query string

	// t_config::iterator it;
	// std::cout << "_________Start________" << std::endl;
    // for (it = request._route->begin(); it != request._route->end(); ++it) {
    //     std::cout << it->first << ": " << it->second << std::endl;
    // }
	// std::cout << "_________END________" << std::endl;
	// std::cout << "-----" << var << "------" << std::endl;
	// std::cout << "-----" << request._uri << "------" << std::endl;
}

std::string	CGI::create_header(void)
{
	std::string header;

	header = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
	size_t size = output.size();
	std::stringstream ss;
	ss << size;
	header = header + ss.str() + "\nConnection: keep-alive\n\n" + output;
	return header;
}