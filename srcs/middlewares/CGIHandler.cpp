#include "CGIHandler.hpp"
#include "Singleton.hpp"
#include "SessionHandler.hpp"
#include "HttpException.hpp"
#include "Util.hpp"
#include <sys/wait.h>

extern char **environ;

std::list<CGI> CGIHandler::_CGI;

void CGIHandler::execute(Request &req, Response &res)
{
    const std::string ext = Util::get_extension(req.get_translated_path());
    std::string arg0 = req.get_location_config().getCgiPath(ext);
    if (arg0.empty())
        return Middleware::execute(req, res);

    if (!Util::file_exists(req.get_translated_path()))
        throw HttpException(404, "Not found");
    
    _CGI.push_back(CGI(res));
    CGI &cgi = _CGI.back();
    cgi.set_session_id(Singleton<SessionHandler>::get_instance().get_session_id());

	cgi.child_pid = fork();
    if (cgi.child_pid == -1)
    {
        perror("fork() failed");
        throw HttpException(500, "fork failed");
    }
    else if (cgi.child_pid == 0)
    {
        char **envp = environ;
        while (*envp)
            cgi.add_local_envp(*envp++);
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

        for (Request::t_headers::const_iterator it = req.get_headers().begin();
                it != req.get_headers().end(); ++it)
        {
            if (it->first.compare(0, 2, "X-") == 0 || it->first.compare("Cookie") == 0)
                cgi.add_envp("HTTP_" + it->first, it->second);
        }
        if (arg0 == "./")
            arg0 = req.get_translated_path();
        cgi.setup_bash(arg0, req.get_translated_path(), req.get_body());
    }
}

void CGIHandler::timeout(size_t execution_timeout_sec)
{
    // just set response to 502
    // select blocks, this function is read only after select times out
    // for convenience is_timeout and select timeout should be the same
    for (std::list<CGI>::iterator it = _CGI.begin(); it != _CGI.end();)
    {
        if (it->is_timeout(execution_timeout_sec))
        {
            it->_response.send_content(502, "Process has timed out");
            kill(it->child_pid, SIGKILL);
            it = _CGI.erase(it);
        }
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
        for (std::list<CGI>::iterator it = _CGI.begin(); it != _CGI.end(); ++it)
        {
            if (it->child_pid == pid)
            {
                it->response(); //todo check error 500 if error/empty, timeout not here but loop_soket()
                _CGI.erase(it);
                break;
            }
        }
    }
}
