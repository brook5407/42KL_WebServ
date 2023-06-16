#include <string>
#include <dirent.h>

#define DT_DIR           4
#define DT_REG           8

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
        void execute(Request &req, Response &)
        {
            DIR *dir;

            if ((*req._route)["index"].empty())
                return;
            dir = opendir(req._script_name.c_str());
            if (dir == NULL)
                return;
            closedir(dir);
            std::string index_file = req._script_name;
            if (req._script_name[req._script_name.size() - 1] != '/')
                index_file += "/";
            index_file += (*req._route)["index"];
            std::ifstream infile(index_file.c_str(), std::ios::in);
            if (infile.is_open())
                req._script_name = index_file;
            std::cout << "test index " << index_file << " scrpt:" << req._script_name << std::endl;
        }
};

class DirectoryListing: public AMiddleware
{
    public:
        void execute(Request &req, Response &res)
        {
            DIR *dir;
            struct dirent *entry;

            dir = opendir(req._script_name.c_str());
            if (dir == NULL)
                return;

            res.write("<html><body>");
            res.write("<h1>Directory listing</h1>");
            res.write("<ol>");

            while ((entry = readdir(dir)) != NULL)
            {
                // hide dot files
                if (entry->d_name[0] == '.')
                    continue;
                if (entry->d_type != DT_DIR && entry->d_type != DT_REG)
                    continue;
                res.write(std::string() + "<li><a href=\"" + req._uri);
                // add directory slash if not present
                if (req._uri[req._uri.size() - 1] != '/')
                    res.write("/");
                res.write(std::string() + entry->d_name);
                // append slash to directory
                if (entry->d_type == DT_DIR)
                    res.write("/");
                res.write(std::string() + "\">" + entry->d_name);
                if (entry->d_type == DT_DIR)
                    res.write("/");
                res.write("</a></li>");
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
            if (stat(req._script_name.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
            {
                res.write_from_file(req._script_name);
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