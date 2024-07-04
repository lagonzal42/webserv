
#pragma once

#include "Parser.hpp"
#include "Request.hpp"
#include "MimeDict.hpp"
#include "Utils.hpp"
#include <iostream>
#include <stack>
#include <iterator>

class ResponseGeneratorPOST
{
	public:
		/*=============================== GET RESPONSES ===================================*/
		static std::string	generatePostResponse(Request& req, const Parser::Location& currentLoc, const Parser::Server& currentServ, std::vector<char *>& envp);
		static std::string	postResponse(Request& req, std::string& cleanPath);
		static std::string	postChunkedResponse(Request& req);
		static std::string	postCgiResponse(const Parser::Location& currentLoc, Request& req, std::vector<char *>& envp, const Parser::Server& currentServ, std::string& cleanPath);
		static std::vector<char *> RequestCopy;
};

std::string extractFileContent(const std::string& requestBody);
std::string getFilename(const std::string &filename);
std::string	processChunks(const std::string& chunkedBody);
