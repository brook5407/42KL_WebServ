#include "IndexHandler.hpp"
#include <dirent.h>

void IndexHandler::execute(Request &req, Response &res)
{
    for (std::size_t i = 0; i < req._location_config->getIndex().size(); ++i)
    {
        const std::string& filename = req._location_config->getIndex()[i];
        DIR *dir = opendir(req._script_name.c_str());
        if (dir)
        {
            closedir(dir);

            std::string index_file = req._script_name;
            if (req._script_name[req._script_name.size() - 1] != '/')
                index_file += "/";
            index_file += filename;
            std::ifstream infile(index_file.c_str());
            if (infile.is_open())
                req._script_name = index_file;
            // std::cout << "test index " << index_file << " scrpt:" << req._script_name << std::endl;
        }
    }
    Middleware::execute(req, res);
}