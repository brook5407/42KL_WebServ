#include "StaticFileHandler.hpp"
#include "HttpException.hpp"
#include "Util.hpp"
#include "MimeType.hpp"

void StaticFileHandler::execute(Request &req, Response &res)
{
    if (Util::file_exists(req.get_translated_path()))
    {
        std::string extension = Util::get_extension(req.get_translated_path());
        MimeType mimetype = req.get_server_config().getMimeTypes();
        std::map<std::string, std::string>::iterator it;
        it = mimetype.find(extension);
        if (it == mimetype.end())
            throw HttpException(403, "Forbidden");
        res.send_file(200, req.get_translated_path(), (*it).second);
    }
    else
        throw HttpException(404, "Not found");
}
