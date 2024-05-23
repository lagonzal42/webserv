#include "MimeDict.hpp"
#include <iostream>

// Static variables that go inside clases need to be defined in a source code file. Declaration and definition is not the same
MimeDict* MimeDict::_dict = NULL;

/**
 * @brief Construct a new Mime Dict:: Mime Dict object
 * 
 */
MimeDict::MimeDict(void)
{
    std::cout << "The mime dict is being created" << std::endl;
    
    _mime[".html"] = "text/html";
    _mime[".txt"] = "text/plain";
    _mime[".css"] = "text/css";
    _mime[".js"] = "text/javascript";
    _mime[".html"] = "text/html";
    _mime[".ico"] = "image/vnd.microsoft.icon";
    _mime[".jpg"] = "image/jpg";
    _mime[".jpeg"] = "image/jpg";
    _mime[".png"] = "image/png";
    _mime[".mp3"] = "audio/mpeg";
    _mime[".mp4"] = "video/mp4";
    _mime[".pdf"] = "application/pdf";
    _mime[".php"] = "application/x-httpd-php";
    _mime[".sh"] = "application/x-sh";
    _mime[".zip"] = "application/zip";
    _mime[".7z"] = "application/x-7z-compressed";
}

MimeDict::~MimeDict(void)
{
    std::cout << "The mime dict was destroyed" << std::endl;
}

MimeDict* MimeDict::getMimeDict(void)
{
    if (!_dict)
        _dict = new MimeDict();
    return _dict;
}

std::map<std::string, std::string>  MimeDict::getMap(void)
{
    return (getMimeDict()->_mime);
}