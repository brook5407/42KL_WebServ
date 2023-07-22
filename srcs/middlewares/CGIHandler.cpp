#include "CGIHandler.hpp"
#include "Singleton.hpp"
#include "SessionHandler.hpp"
#include "HttpException.hpp"
#include "Util.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>

extern char **environ;

std::list<CGI> CGIHandler::_cgi_processes;

void CGIHandler::execute(Request &req, Response &res)
{
    const std::string ext = Util::get_extension(req.get_translated_path());
    std::string arg0 = req.get_location_config().getCgiPath(ext);
    if (arg0.empty())
        return Middleware::execute(req, res);
    if (arg0 == "./")
        arg0 = req.get_translated_path();
    else
        arg0 = Util::absolute_path(arg0);
    if (!Util::file_exists(arg0))
        throw HttpException(404, "Cgi not found " + arg0);
    if (!Util::file_exists(req.get_translated_path()))
        throw HttpException(404, "Cgi not found " + req.get_translated_path());
    if (!Util::file_executable(arg0))
        throw HttpException(403, "require executable file " + arg0);

    CGI cgi(req, res);
	pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork() failed");
        throw HttpException(500, "fork failed");
    }
    else if (pid > 0)
    {
        cgi.set_pid(pid);
        cgi.set_session_id(Singleton<SessionHandler>::get_instance().get_session_id());
        _cgi_processes.push_back(cgi);
    }
    else
    {
        chdir(Util::dirname(req.get_translated_path()).c_str());
        // duplicate environment is replaced by newer value
        for (Request::t_headers::const_iterator it = req.get_headers().begin(); it != req.get_headers().end(); ++it)
            cgi.add_envp("HTTP_" + it->first, it->second);
        for (char **envp = environ; *envp; ++envp)
            cgi.add_local_envp(*envp);
        cgi.add_envp("REQUEST_METHOD", req.get_method());
        cgi.add_envp("SERVER_PROTOCOL", "HTTP/1.1");
        cgi.add_envp("PATH_INFO", req.get_uri());
        cgi.add_envp("QUERY_STRING", req.get_query_string());
        cgi.add_envp("HTTP_SESSION", Singleton<SessionHandler>::get_instance().get_session());
        cgi.add_envp("CONTENT_LENGTH", Util::to_string(req.get_body_length()));
        cgi.add_envp("CONTENT_TYPE", req.get_header("Content-Type"));
        cgi.add_envp("GATEWAY_INTERFACE", "CGI/1.1");
        cgi.add_envp("REQUESTED_URI", req.get_uri());
        cgi.add_envp("REMOTE_ADDR", res._connection._client_ip);
        // cgi.add_envp("SCRIPT_NAME", req.get_translated_path());
        cgi.add_envp("SERVER_SOFTWARE", "webserv");
        cgi.add_envp("SERVER_PORT", Util::to_string(res._connection._server_port));
        cgi.exec(arg0, req.get_translated_path(), req.get_body());
    }
}

void CGIHandler::timeout(size_t execution_timeout_sec)
{
    // just set response to 502
    // select blocks, this function is read only after select times out
    // for convenience is_timeout and select timeout should be the same
    for (std::list<CGI>::iterator it = _cgi_processes.begin(); it != _cgi_processes.end();)
    {
        if (it->timeout(execution_timeout_sec))
            it = _cgi_processes.erase(it);
        else
            ++it;
    }
}

// verify kill server, kill cgi
void CGIHandler::handle_exit(void)
{
    int     status;
    pid_t   pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (std::list<CGI>::iterator it = _cgi_processes.begin(); it != _cgi_processes.end(); ++it)
        {
            if (it->get_pid() == pid)
            {
                if (WIFEXITED(status))
                    status = WEXITSTATUS(status);
                else if (WIFSIGNALED(status))
                    status = 128 + WTERMSIG(status);
                it->response(status);
                _cgi_processes.erase(it);
                break;
            }
        }
    }
}
