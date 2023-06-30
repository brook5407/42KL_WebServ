#ifndef LIMITREQUESTBODYHANDLER_HPP
# define LIMITREQUESTBODYHANDLER_HPP

class LimitRequestBodyHandler : public Middleware
{
    public:
        void execute(Request &req, Response &res)
        {
            if (req._body.size() > req._server_config->getMaxBodySize())
                throw HttpException(413, "Request Entity Too Large");
            Middleware::execute(req, res);
        }
};

#endif
