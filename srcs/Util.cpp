#include "Util.hpp"
#include <unistd.h>
#include <sys/stat.h>

bool	Util::file_exists(const std::string &filepath)
{
    struct stat sb;
    return stat(filepath.c_str(), &sb) == 0 && S_ISREG(sb.st_mode);
}

bool   Util::file_executable(const std::string &filepath)
{
    return (access(filepath.c_str(), X_OK) == 0);
}

bool    Util::file_extension(const std::string &filepath, const std::string &extension)
{
    return (filepath.size() > extension.size()
        && filepath.find(extension, filepath.size() - extension.size()) != std::string::npos);
    // return (filepath.substr(filepath.find_last_of('.')) == extension);
}

std::string Util::get_extension(const std::string &path)
{
    std::size_t pos = path.find_last_of('.');
    if (pos == std::string::npos)
        return std::string();
    return path.substr(pos);
}

std::string Util::combine_path(const std::string &path1, const std::string &path2)
{
    std::string path = path1;
    if (path[path.size() - 1] != '/')
        path += '/';
    path += path2;
    return path;
}
