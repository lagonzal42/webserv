
#pragma once

#include "Parser.hpp"
#include "Request.hpp"
#include "MimeDict.hpp"
#include "Utils.hpp"
#include <iostream>

class ResponseGenerator
{
	public:
		/*=============================== GET RESPONSES ===================================*/
		static const char	*generateGetResponse(Request& req, const Parser::Location& currentLoc, const Parser::Server& currentServ, std::vector<char *>& envp);
		static const char	*errorResponse(int errorCode, const Parser::Server& currentServ);
		static const char	*getRedirResponse(); //I still dont know what do i need here, i supose request is needed but idk if something else
		static const char	*getCgiResponse(const Parser::Location& currentLoc, Request& req, std::vector<char *>& envp, const Parser::Server& currentServ, std::string& cleanPath);
		static const char	*getAutoindexResponse(const Parser::Server& currentServ, std::string& cleanPath);
		static const char	*getFileResponse(const Parser::Location& currentLoc, const Parser::Server& currentServ, std::string& cleanPath);
		static std::string	parsePath(std::string servPath, std::string locPath, std::string reqPath);
};