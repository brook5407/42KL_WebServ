#include "IndexHandler.hpp"
#include "Util.hpp"
#include <dirent.h>

void IndexHandler::execute(Request &req, Response &res)
{
    DIR *dir = opendir(req._script_name.c_str());
    if (dir)
    {
        closedir(dir);

        for (std::size_t i = 0; i < req._location_config->getIndex().size(); ++i)
        {
            const std::string &filename = req._location_config->getIndex()[i];
            const std::string index_filepath = Util::combine_path(req._script_name, filename);
            if (Util::file_exists(index_filepath))
            {
                req._script_name = index_filepath;
                break;
            }
        }
    }
    Middleware::execute(req, res);
}
