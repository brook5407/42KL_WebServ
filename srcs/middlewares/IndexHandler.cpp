#include "IndexHandler.hpp"
#include "Util.hpp"
#include <dirent.h>

void IndexHandler::execute(Request &req, Response &res)
{
    if (Util::dir_exists(req.get_translated_path()))
    {
        for (std::size_t i = 0; i < req.get_location_config().getIndex().size(); ++i)
        {
            const std::string &filename = req.get_location_config().getIndex()[i];
            const std::string index_filepath = Util::combine_path(req.get_translated_path(), filename);
            if (Util::file_exists(index_filepath))
            {
                req.set_translated_path(index_filepath);
                break;
            }
        }
    }
    Middleware::execute(req, res);
}
