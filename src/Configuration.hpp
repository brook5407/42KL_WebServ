#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "ConfigParser.hpp"
#include <map>
#include <string>

typedef std::map<std::string, std::string> t_config;
typedef std::map<std::string, t_config > t_configs;
typedef t_configs::iterator t_configs_it;

class Configuration
{
public:
    Configuration()
        : _reason_phrase(), _mime_types()
    {
        load_reason_phrases();
        load_mime_types();
    }
    std::map<int, const char *> _reason_phrase;
    std::map<std::string, std::string> _mime_types;
private:
    void load_reason_phrases()
    {
        _reason_phrase[100] = "Continue";
        _reason_phrase[101] = "Switching Protocols";
        _reason_phrase[102] = "Processing";
        _reason_phrase[200] = "OK";
        _reason_phrase[201] = "Created";
        _reason_phrase[202] = "Accepted";
        _reason_phrase[203] = "Non-Authoritative Information";
        _reason_phrase[204] = "No Content";
        _reason_phrase[205] = "Reset Content";
        _reason_phrase[206] = "Partial Content";
        _reason_phrase[207] = "Multi-Status";
        _reason_phrase[208] = "Already Reported";
        _reason_phrase[226] = "IM Used";
        _reason_phrase[300] = "Multiple Choices";
        _reason_phrase[301] = "Moved Permanently";
        _reason_phrase[302] = "Found";
        _reason_phrase[303] = "See Other";
        _reason_phrase[304] = "Not Modified";
        _reason_phrase[305] = "Use Proxy";
        _reason_phrase[306] = "Switch Proxy";
        _reason_phrase[307] = "Temporary Redirect";
        _reason_phrase[308] = "Permanent Redirect";
        _reason_phrase[400] = "Bad Request";
        _reason_phrase[401] = "Unauthorized";
        _reason_phrase[402] = "Payment Required";
        _reason_phrase[403] = "Forbidden";
        _reason_phrase[404] = "Not Found";
        _reason_phrase[405] = "Method Not Allowed";
        _reason_phrase[406] = "Not Acceptable";
        _reason_phrase[407] = "Proxy Authentication Required";
        _reason_phrase[408] = "Request Timeout";
        _reason_phrase[409] = "Conflict";
        _reason_phrase[410] = "Gone";
        _reason_phrase[411] = "Length Required";
        _reason_phrase[412] = "Precondition Failed";
        _reason_phrase[413] = "Payload Too Large";
        _reason_phrase[414] = "URI Too Long";
        _reason_phrase[415] = "Unsupported Media Type";
        _reason_phrase[416] = "Range Not Satisfiable";
        _reason_phrase[417] = "Expectation Failed";
        _reason_phrase[418] = "I'm a teapot";
        _reason_phrase[421] = "Misdirected Request";
        _reason_phrase[422] = "Unprocessable Entity";
        _reason_phrase[423] = "Locked";
        _reason_phrase[424] = "Failed Dependency";
        _reason_phrase[426] = "Upgrade Required";
        _reason_phrase[428] = "Precondition Required";
        _reason_phrase[429] = "Too Many Requests";
        _reason_phrase[431] = "Request Header Fields Too Large";
        _reason_phrase[451] = "Unavailable For Legal Reasons";
        _reason_phrase[500] = "Internal Server Error";
        _reason_phrase[501] = "Not Implemented";
        _reason_phrase[502] = "Bad Gateway";
        _reason_phrase[503] = "Service Unavailable";
        _reason_phrase[504] = "Gateway Timeout";
        _reason_phrase[505] = "HTTP Version Not Supported";
        _reason_phrase[506] = "Variant Also Negotiates";
        _reason_phrase[507] = "Insufficient Storage";
        _reason_phrase[508] = "Loop Detected";
        _reason_phrase[510] = "Not Extended";
        _reason_phrase[511] = "Network Authentication Required";
        _reason_phrase[520] = "Unknown Error";
        _reason_phrase[522] = "Origin Connection Time-out";
        _reason_phrase[598] = "Network read timeout error";
        _reason_phrase[599] = "Network connect timeout error";
    }
    void load_mime_types()
    {
        _mime_types["aac"] = "audio/aac";
        _mime_types["abw"] = "application/x-abiword";
        _mime_types["arc"] = "application/octet-stream";
        _mime_types["avi"] = "video/x-msvideo";
        _mime_types["azw"] = "application/vnd.amazon.ebook";
        _mime_types["bin"] = "application/octet-stream";
        _mime_types["bz"] = "application/x-bzip";
        _mime_types["bz2"] = "application/x-bzip2";
        _mime_types["csh"] = "application/x-csh";
        _mime_types["css"] = "text/css";
        _mime_types["csv"] = "text/csv";
        _mime_types["doc"] = "application/msword";
        _mime_types["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        _mime_types["eot"] = "application/vnd.ms-fontobject";
        _mime_types["epub"] = "application/epub+zip";
        _mime_types["gif"] = "image/gif";
        _mime_types["htm"] = "text/html";
        _mime_types["html"] = "text/html";
        _mime_types["ico"] = "image/x-icon";
        _mime_types["ics"] = "text/calendar";
        _mime_types["jar"] = "application/java-archive";
        _mime_types["jpeg"] = "image/jpeg";
        _mime_types["jpg"] = "image/jpeg";
        _mime_types["js"] = "application/javascript";
        _mime_types["json"] = "application/json";
        _mime_types["mid"] = "audio/midi";
        _mime_types["midi"] = "audio/midi";
        _mime_types["mp4"] = "video/mp4";
        _mime_types["mpeg"] = "video/mpeg";
        _mime_types["mpkg"] = "application/vnd.apple.installer+xml";
        _mime_types["odp"] = "application/vnd.oasis.opendocument.presentation";
        _mime_types["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
        _mime_types["odt"] = "application/vnd.oasis.opendocument.text";
        _mime_types["oga"] = "audio/ogg";
        _mime_types["ogv"] = "video/ogg";
        _mime_types["ogx"] = "application/ogg";
        _mime_types["otf"] = "font/otf";
        _mime_types["png"] = "image/png";
        _mime_types["pdf"] = "application/pdf";
        _mime_types["ppt"] = "application/vnd.ms-powerpoint";
        _mime_types["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
        _mime_types["rar"] = "application/x-rar-compressed";
        _mime_types["rtf"] = "application/rtf";
        _mime_types["sh"] = "application/x-sh";
        _mime_types["svg"] = "image/svg+xml";
        _mime_types["swf"] = "application/x-shockwave-flash";
        _mime_types["tar"] = "application/x-tar";
        _mime_types["tif"] = "image/tiff";
        _mime_types["tiff"] = "image/tiff";
        _mime_types["ts"] = "application/typescript";
        _mime_types["ttf"] = "font/ttf";
        _mime_types["vsd"] = "application/vnd.visio";
        _mime_types["wav"] = "audio/x-wav";
        _mime_types["weba"] = "audio/webm";
        _mime_types["webm"] = "video/webm";
        _mime_types["webp"] = "image/webp";
        _mime_types["woff"] = "font/woff";
        _mime_types["woff2"] = "font/woff2";
        _mime_types["xhtml"] = "application/xhtml+xml";
        _mime_types["xls"] = "application/vnd.ms-excel";
        _mime_types["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        _mime_types["xml"] = "application/xml";
        _mime_types["xul"] = "application/vnd.mozilla.xul+xml";
        _mime_types["zip"] = "application/zip";
        _mime_types["3gp"] = "video/3gpp";
        _mime_types["3g2"] = "video/3gpp2";
        _mime_types["7z"] = "application/x-7z-compressed";
    }
};
#endif
