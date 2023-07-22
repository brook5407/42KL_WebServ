#include "KeepAliveHandler.hpp"

void KeepAliveHandler::execute(Request &req, Response &res)
{
    res.set_keep_alive(req.get_header("Connection") != "close");
    Middleware::execute(req, res);
}
