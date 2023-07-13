#include "MimeType.hpp"

MimeType::MimeType()
{
    // operator[](".aac") = "audio/aac";
    // operator[](".abw") = "application/x-abiword";
    // operator[](".arc") = "application/octet-stream";
    // operator[](".avi") = "video/x-msvideo";
    // operator[](".azw") = "application/vnd.amazon.ebook";
    // operator[](".bin") = "application/octet-stream";
    // operator[](".bz") = "application/x-bzip";
    // operator[](".bz2") = "application/x-bzip2";
    // operator[](".csh") = "application/x-csh";
    // operator[](".css") = "text/css";
    // operator[](".csv") = "text/csv";
    // operator[](".doc") = "application/msword";
    // operator[](".docx") = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    // operator[](".eot") = "application/vnd.ms-fontobject";
    // operator[](".epub") = "application/epub+zip";
    // operator[](".gif") = "image/gif";
    // operator[](".htm") = "text/html";
    // operator[](".html") = "text/html";
    // operator[](".ico") = "image/x-icon";
    // operator[](".ics") = "text/calendar";
    // operator[](".jar") = "application/java-archive";
    // operator[](".jpeg") = "image/jpeg";
    // operator[](".jpg") = "image/jpeg";
    // operator[](".js") = "application/javascript";
    // operator[](".json") = "application/json";
    // operator[](".mid") = "audio/midi";
    // operator[](".midi") = "audio/midi";
    // operator[](".mp4") = "video/mp4";
    // operator[](".mpeg") = "video/mpeg";
    // operator[](".mpkg") = "application/vnd.apple.installer+xml";
    // operator[](".odp") = "application/vnd.oasis.opendocument.presentation";
    // operator[](".ods") = "application/vnd.oasis.opendocument.spreadsheet";
    // operator[](".odt") = "application/vnd.oasis.opendocument.text";
    // operator[](".oga") = "audio/ogg";
    // operator[](".ogv") = "video/ogg";
    // operator[](".ogx") = "application/ogg";
    // operator[](".otf") = "font/otf";
    // operator[](".png") = "image/png";
    // operator[](".pdf") = "application/pdf";
    // operator[](".ppt") = "application/vnd.ms-powerpoint";
    // operator[](".pptx") = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    // operator[](".rar") = "application/x-rar-compressed";
    // operator[](".rtf") = "application/rtf";
    // operator[](".sh") = "application/x-sh";
    // operator[](".svg") = "image/svg+xml";
    // operator[](".swf") = "application/x-shockwave-flash";
    // operator[](".tar") = "application/x-tar";
    // operator[](".tif") = "image/tiff";
    // operator[](".tiff") = "image/tiff";
    // operator[](".ts") = "application/typescript";
    // operator[](".ttf") = "font/ttf";
    // operator[](".vsd") = "application/vnd.visio";
    // operator[](".wav") = "audio/x-wav";
    // operator[](".weba") = "audio/webm";
    // operator[](".webm") = "video/webm";
    // operator[](".webp") = "image/webp";
    // operator[](".woff") = "font/woff";
    // operator[](".woff2") = "font/woff2";
    // operator[](".xhtml") = "application/xhtml+xml";
    // operator[](".xls") = "application/vnd.ms-excel";
    // operator[](".xlsx") = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    // operator[](".xml") = "application/xml";
    // operator[](".xul") = "application/vnd.mozilla.xul+xml";
    // operator[](".zip") = "application/zip";
    // operator[](".3gp") = "video/3gpp";
    // operator[](".3g2") = "video/3gpp2";
    // operator[](".7z") = "application/x-7z-compressed";
}

std::string MimeType::lookup(const std::string &extension) const
{
    if (count(extension))
        return find(extension)->second;
    return std::string();
}
