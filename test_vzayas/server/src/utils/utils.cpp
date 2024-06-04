#include "../../inc/webserver.h"

// Helper function to extract filename from Content-Disposition header
std::string getFilename(const std::string &contentDisposition) {
    size_t pos = contentDisposition.find("filename=");
    if (pos == std::string::npos) {
        return "";
    }
    size_t start = contentDisposition.find('"', pos) + 1;
    size_t end = contentDisposition.find('"', start);
    return contentDisposition.substr(start, end - start);
}

// std::string base64_encode(const std::string &input) {
//     std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//     std::string encoded;
//     int val = 0, valb = -6;
//     for (std::string::const_iterator it = input.begin(); it != input.end(); it++)
//     {
//         val = (val << 8) + static_cast<unsigned char>(*it);
//         valb += 8;
//         while (valb >= 0)
//         {
//             encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
//             valb -= 6;
//         }
//     }
//     if (valb > -6)
//     {
//         encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
//     }
//     while (encoded.size() % 4 != 0)
//     {
//         encoded.push_back('=');
//     }
//     return encoded;
// }