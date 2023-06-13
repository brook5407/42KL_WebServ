#include <string>
#include <dirent.h>

//abstract class
class Middleware
{
    public:
        virtual void execute(Request &, Response &) = 0;

    private:
        std::map<std::string, std::string> _config;
};

class IndexFile : public Middleware
{
    public:
        void execute(Request &, Response &)
        {
        }
};

class DirectoryListing: public Middleware
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

class Redirect: public Middleware
{
    public:
        void execute(Request &, Response &res)
        {
            res.status(301);
            res.header("Location", "http://www.google.com");
            res.end();
        }
};

class StaticFile: public Middleware
{
    public:
        // todo add header last-modified
        void execute(Request &req, Response &res)
        {
            // avoid "GET / HTTP/1.1" infinity loop
            struct stat sb;
            if (!(stat(("./wwwroot/" + req._uri).c_str(), &sb) == 0 && S_ISREG(sb.st_mode)))
                return;

            // int fd = open(("./wwwroot/" + req._uri).c_str(), O_RDONLY);
            // if (fd == -1 || read(fd, 0, 0) == -1)
            //     return;
            // close(fd);

            // res.header("Content-Type", "text/html"); //todo by file extension
            res.write_from_file("./wwwroot/" + req._uri);
            res.end();
        }
};

class ErrorPage: public Middleware
{
    public:
        void execute(Request &, Response &res)
        {
            if (res.status() == 200)
                res.status(404);
            std::stringstream ss;
            ss <<  "./error_pages/" << res.status() << ".html";
            std::cout << "error page: " << ss.str() << std::endl;
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
