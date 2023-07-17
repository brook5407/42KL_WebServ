#ifndef REASONPHRASE_HPP
# define REASONPHRASE_HPP

# include <map>

class ReasonPhrase: public std::map<int, const char *>
{
    public:
        ReasonPhrase();
        const char *lookup(int status_code);
};

#endif
