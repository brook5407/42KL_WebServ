#include <string>
#include <dirent.h>

//abstract class
class AMiddleware
{
    public:
        virtual void execute(Request &, Response &) = 0;

    private:
        std::map<std::string, std::string> _config;
};

// parser
// if value is not in "GET|DEL...|Get" throw exception("invalid method value")

class CheckMethod : public AMiddleware
{
    public:
        void execute(Request &, Response &)
        {
            // get allowed method by route
            // if (method not allowed)
            // {
            //     res.status(405);
            //     res.end();
            // }
        }
};

class IndexFile : public AMiddleware
{
    public:
        void execute(Request &, Response &)
        {
        }
};

class DirectoryListing: public AMiddleware
{
    public:
        void execute(Request &req, Response &res)
        {
            // stringstream
            // http://server/dir/
            //config["root"] + uri
            if (req._uri[req._uri.size() - 1] != '/')
                return;

            DIR *dir;
            struct dirent *entry;

            // Open directory
            dir = opendir("./wwwroot/");
            if (dir == NULL) {
                perror("opendir");
                return;
            }

            res.write("<html><body>");
            res.write("<h1>Directory listing</h1>");
            res.write("<ol>");

            // Read directory entries
            while ((entry = readdir(dir)) != NULL)
            {
                res.write(std::string("<li><a href=\"/") + entry->d_name + "\">" + entry->d_name + "</a></li>");
            }
            closedir(dir);

            res.write("</ol>");
            res.write("</html></body>");
            res.end();
        }
};

class Redirect: public AMiddleware
{
    public:
        void execute(Request &, Response &res)
        {
            res.status(301);
            res.header("Location", "http://www.google.com");
            res.end();
        }
};

class StaticFile: public AMiddleware
{
    public:
        void execute(Request &req, Response &res)
        {
            struct stat sb;
            //todo test permission
            if (stat(("./wwwroot/" + req._uri).c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
            {
                res.write_from_file("./wwwroot/" + req._uri);
                res.end();
            }
            //todo 404 with error page instead of continue to next middleware
        }
};

// todo this should be part of Response, instead of middleware?
class ErrorPage: public AMiddleware
{
    public:
        void execute(Request &req, Response &res)
        {
            if (res.status() == 0)
                res.status(404);
            std::stringstream ss;
            ss <<  "./error_pages/" << res.status() << ".html";
            std::cout << "error page: " << ss.str() << " for " << req._uri << std::endl;
            res.write_from_file(ss.str());
            res.end();
        }
};

template <typename T>
struct Singleton
{
    static T *get_instance()
    {
        static T instance;
        return &instance;
    }
};

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os;
    os << value;
    return os.str();
}