#ifndef MIMETYPE_HPP
# define MIMETYPE_HPP

#include <string>
#include <map>

class MimeType: public std::map<std::string, std::string>
{
    public:
        MimeType();
};

#endif
