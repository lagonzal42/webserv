
#pragma once

#include "Parser.hpp"
#include "Request.hpp"
#include "MimeDict.hpp"
#include "Utils.hpp"
#include <iostream>

class ResponseGeneratorPOST
{
	public:
		/*=============================== GET RESPONSES ===================================*/
		static std::string	generatePostResponse(Request& req, const Parser::Location& currentLoc, const Parser::Server& currentServ, std::vector<char *>& envp);
		static std::string	postResponse(const Parser::Location& currentLoc, Request& req, std::vector<char *>& envp, const Parser::Server& currentServ, std::string& cleanPath);
		static std::string	postChunkedResponse(const Parser::Location& currentLoc, Request& req, const Parser::Server& currentServ, std::string& cleanPath);
		static std::string	postCgiResponse(const Parser::Location& currentLoc, Request& req, std::vector<char *>& envp, const Parser::Server& currentServ, std::string& cleanPath);
		static std::string	parsePath(std::string servPath, std::string locPath, std::string reqPath);
		static std::string	errorResponse(int errorCode, const Parser::Server& currentServ);
};

std::string extractFileContent(const std::string& requestBody);
std::string getFilename(const std::string &filename);