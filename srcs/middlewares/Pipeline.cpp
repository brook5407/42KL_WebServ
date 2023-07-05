#include "Pipeline.hpp"
#include "Singleton.hpp"
#include "AutoIndexHandler.hpp"
#include "CGIHandler.hpp"
#include "ErrorHandler.hpp"
#include "AllowMethodHandler.hpp"
#include "IndexHandler.hpp"
#include "RedirectHandler.hpp"
#include "StaticFileHandler.hpp"
#include "UploadHandler.hpp"
#include "LimitRequestBodyHandler.hpp"
#include "SessionHandler.hpp"

static void add_handler(Middleware *pipeline, Middleware *middleware);

Pipeline::Pipeline()
{
    add_handler(this, Singleton<ErrorHandler>::get_instance());
    add_handler(this, Singleton<SessionHandler>::get_instance());
    add_handler(this, Singleton<Logger>::get_instance());
    add_handler(this, Singleton<KeepAliveHandler>::get_instance());
    add_handler(this, Singleton<AllowMethodHandler>::get_instance());
    add_handler(this, Singleton<RedirectHandler>::get_instance());
    add_handler(this, Singleton<LimitRequestBodyHandler>::get_instance());
    add_handler(this, Singleton<IndexHandler>::get_instance()); // all methods
    add_handler(this, Singleton<CGIHandler>::get_instance()); // all methods: upload.cgi?
    add_handler(this, Singleton<UploadHandler>::get_instance()); // POST,PUT,DELETE must after cgi
    add_handler(this, Singleton<AutoIndexHandler>::get_instance()); //GET
    add_handler(this, Singleton<StaticFileHandler>::get_instance()); //GET
};

void add_handler(Middleware *pipeline, Middleware *middleware)
{
    Middleware *walk = pipeline;
    while (walk->getNext())
        walk = walk->getNext();
    walk->setNext(middleware);
}
