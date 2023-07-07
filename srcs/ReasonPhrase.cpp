#include "ReasonPhrase.hpp"

ReasonPhrase::ReasonPhrase()
{
    operator[](100) = "Continue";
    operator[](101) = "Switching Protocols";
    operator[](102) = "Processing";
    operator[](200) = "OK";
    operator[](201) = "Created";
    operator[](202) = "Accepted";
    operator[](203) = "Non-Authoritative Information";
    operator[](204) = "No Content";
    operator[](205) = "Reset Content";
    operator[](206) = "Partial Content";
    operator[](207) = "Multi-Status";
    operator[](208) = "Already Reported";
    operator[](226) = "IM Used";
    operator[](300) = "Multiple Choices";
    operator[](301) = "Moved Permanently";
    operator[](302) = "Found";
    operator[](303) = "See Other";
    operator[](304) = "Not Modified";
    operator[](305) = "Use Proxy";
    operator[](306) = "Switch Proxy";
    operator[](307) = "Temporary Redirect";
    operator[](308) = "Permanent Redirect";
    operator[](400) = "Bad Request";
    operator[](401) = "Unauthorized";
    operator[](402) = "Payment Required";
    operator[](403) = "Forbidden";
    operator[](404) = "Not Found";
    operator[](405) = "Method Not Allowed";
    operator[](406) = "Not Acceptable";
    operator[](407) = "Proxy Authentication Required";
    operator[](408) = "Request Timeout";
    operator[](409) = "Conflict";
    operator[](410) = "Gone";
    operator[](411) = "Length Required";
    operator[](412) = "Precondition Failed";
    operator[](413) = "Payload Too Large";
    operator[](414) = "URI Too Long";
    operator[](415) = "Unsupported Media Type";
    operator[](416) = "Range Not Satisfiable";
    operator[](417) = "Expectation Failed";
    operator[](418) = "I'm a teapot";
    operator[](421) = "Misdirected Request";
    operator[](422) = "Unprocessable Entity";
    operator[](423) = "Locked";
    operator[](424) = "Failed Dependency";
    operator[](426) = "Upgrade Required";
    operator[](428) = "Precondition Required";
    operator[](429) = "Too Many Requests";
    operator[](431) = "Request Header Fields Too Large";
    operator[](451) = "Unavailable For Legal Reasons";
    operator[](500) = "Internal Server Error";
    operator[](501) = "Not Implemented";
    operator[](502) = "Bad Gateway";
    operator[](503) = "Service Unavailable";
    operator[](504) = "Gateway Timeout";
    operator[](505) = "HTTP Version Not Supported";
    operator[](506) = "Variant Also Negotiates";
    operator[](507) = "Insufficient Storage";
    operator[](508) = "Loop Detected";
    operator[](510) = "Not Extended";
    operator[](511) = "Network Authentication Required";
    operator[](520) = "Unknown Error";
    operator[](522) = "Origin Connection Time-out";
    operator[](598) = "Network read timeout error";
    operator[](599) = "Network connect timeout error";
}

const char *ReasonPhrase::lookup(int status_code)
{
    if (count(status_code))
        return operator[](status_code);
    return "Unknown";
}
