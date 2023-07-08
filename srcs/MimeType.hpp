#ifndef MIMETYPE_HPP
# define MIMETYPE_HPP

#include <string>
#include <map>

class MimeType: public std::map<std::string, const char *>
{
    public:
        MimeType();
        const char *lookup(const std::string &extension);
};

#endif
