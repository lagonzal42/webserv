#include "../../inc/webserver.h"

std::string parseChunked(std::string request)
{
	std::string req = request;
	std::string start = request.substr(0, request.find("\r\n\r\n"));

	if (start.empty()) {
		std::cerr << "Invalid request" << std::endl;
		return "";
	}

	start += 4;

	while (true) {
		std::string end;
		unsigned long chunksize = strtoul(start.c_str(), NULL, 16);

		if (chunksize == 0) {
			break;
		}

		start = *(end.begin() + 2); //Skip chunk size and CRLF
		req.append(start, chunksize); //Append chunk data to request
		start += chunksize + 2; //Skip chunk size and CRLF
	}

	std::cout << "\n ES CHUNKED!!! : \n";
    std::cout << req << std::endl;
    std::cout << "DATA LEN: ";
    std::cout << req.length() << std::endl;
    return req;
}