#ifndef SINGLETON_HPP
# define SINGLETON_HPP

template <typename T>
struct Singleton
{
    static T &get_instance()
    {
        static T instance;
        return instance;
    }
};

#endif
