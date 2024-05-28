#include "MimeDict.hpp"
#include <iostream>

int main(void)
{
    MimeDict* MimeDict = MimeDict::getMimeDict();
    std::map<std::string, std::string> mime = MimeDict->getMap();

    std::string extension = ".css";

    std::cout << "The content type for " << extension << " is " << mime[extension] << std::endl;
    delete MimeDict;
}