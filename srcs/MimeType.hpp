#ifndef MIMETYPE_HPP
# define MIMETYPE_HPP

#include <map>

class MimeType: public std::map<const char *, const char *>
{
    public:
        MimeType();
        const char *lookup(const char *extension);
};

#endif
