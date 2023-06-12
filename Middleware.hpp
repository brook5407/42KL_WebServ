#include <string>

class Middleware
{
    public:
        virtual void execute(Request, Response) = 0;
        //{ std::cout <<"virtual" <<std::endl; };//
    private:
        std::map<std::string, std::string> _config;
};

class IndexFile : public Middleware
{
    public:
        void execute(Request req, Response )
        {
            req._uri = "/index.html";
        }
};

class StaticFile: public Middleware
{
    public:
        void execute(Request req, Response res)
        {
            //root directive
            std::cout << "static file " << req._uri << std::endl;
            int fd = open(("." + req._uri).c_str(), O_RDONLY);
            char buffer[1024];
            int length;
            int total = 0;

            // std::fstream file("." + req._uri, std::ios::in | std::ios::binary);
            // //read file stream to string
            // std::string 

            std::string contents;
            while ((length = read(fd, buffer, sizeof(buffer))) > 0)
            {
                total += length;
                contents += std::string(buffer, length);
            }
            res.write(
                std::string("HTTP/1.1 200 OK") 
        + 
            "\r\n\r\n" + contents);
        }
};

class NotFound: public Middleware
{
    public:
        void execute(Request, Response res)
        {

            res.write("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
        }
};