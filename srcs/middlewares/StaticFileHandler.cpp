#include "StaticFileHandler.hpp"
#include "HttpException.hpp"
#include "Util.hpp"
#include "MimeType.hpp"

void StaticFileHandler::execute(Request &req, Response &res)
{
    if (Util::file_exists(req._script_name))
    {
        std::string extension = Util::get_extension(req._script_name);
        std::map<std::string, std::string>::iterator it;
        MimeType mimetype = req._server_config->getMimeTypes();
        for (it = mimetype.begin(); it != mimetype.end(); ++it)
        {
            std::cout << "the mimetypes are: " << (*it).first << " and " << (*it).second << std::endl;
            if (extension == (*it).first)
                break;
        }
        if (it == mimetype.end())
            throw HttpException(403, "Forbidden");
        res.send_file(200, req._script_name, (*it).second);
    }
    else
        throw HttpException(404, "Not found");
}
