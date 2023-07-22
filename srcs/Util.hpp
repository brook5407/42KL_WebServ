#ifndef UTIL_HPP
# define UTIL_HPP

# include <string>
# include <sstream>

namespace Util
{
    bool	file_exists(const std::string &filepath);
    bool    file_executable(const std::string &filepath);
    bool    file_extension(const std::string &filepath, const std::string &extension);
    std::string get_extension(const std::string &path);
    std::string combine_path(const std::string &path1, const std::string &path2);
    bool	dir_exists(const std::string &filepath);
    std::string dirname(const std::string& filePath);
    std::string absolute_path(const std::string &path);

    template <typename T>
    std::string to_string(T value)
    {
        std::ostringstream os;
        os << value;
        return os.str();
    }
}

#endif
