#include "CGIHandler.hpp"

extern char **environ;

void CGIHandler::execute(Request &req, Response &res)
{
    for (std::size_t i = 0; i < req._location_config->checkCgiExtension().size(); ++i)
    {
        const std::string &extension = req._location_config->checkCgiExtension()[i];
        if (file_extension(req._script_name, extension))
        {
            // if (!file_exists(req._script_name))
            //     throw HttpException(404, "File not found");
            // if (!file_executable(req._script_name))
            //     throw HttpException(403, "File is not executable");

            _CGI.push_back(CGI(res));
            CGI &cgi = _CGI.back();
            // char **envp = environ;
            // while (*envp)
            //     cgi.add_envp(*envp++);
            cgi.add_envp("REQUEST_METHOD", req._method);
            cgi.add_envp("SERVER_PROTOCOL", "HTTP/1.1");
            cgi.add_envp("PATH_INFO", req._uri);
            cgi.add_envp("QUERY_STRING", req._search);

            // cgi.add_envp("CONTENT_LENGTH", to_string(req._content_length));
            // cgi.add_envp("CONTENT_TYPE", req._headers["Content-Type"]);
            // cgi.add_envp("GATEWAY_INTERFACE", "CGI/1.1");
            // cgi.add_envp("PATH_INFO", req._uri);
            // cgi.add_envp("PATH_TRANSLATED", req._script_name);
            // cgi.add_envp("REQUESTED_URI", req._uri);
            // cgi.add_envp("REMOTE_ADDR", res._connection._client_ip);
            // cgi.add_envp("SCRIPT_NAME", req._script_name);
            // cgi.add_envp("SERVER_SOFTWARE", "webserv");
            // cgi.add_envp("SERVER_PORT", to_string(res._connection._server_port));

            for (std::map<std::string, std::string>::iterator it = req._headers.begin();
                    it != req._headers.end(); ++it)
            {
                if (it->first.find("X-") == 0 || it->first.find("Cookie") == 0)
                    cgi.add_envp("HTTP_" + it->first, it->second);
            }
            if (extension == ".bla")
            {
                #if __APPLE__ && __MACH__
                cgi.setup_bash("cgi_tester", req._script_name, req._body);
                #else
                cgi.setup_bash("ubuntu_cgi_tester", req._script_name, req._body);
                #endif
            }
            else if (extension == ".py")
                cgi.setup_bash("/usr/local/bin/python3", req._script_name, req._body);
            else if (extension == ".sh")
                cgi.setup_bash("/bin/bash", req._script_name, req._body);
            else
                cgi.setup_bash(req._script_name, req._script_name, req._body);
            return;
        }
    }
    Middleware::execute(req, res);
}