
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
		static std::string	generateGetResponse(Request& req, const Parser::Location& currentLoc, const Parser::Server& currentServ, std::vector<char *>& envp);
		static std::string	errorResponse(int errorCode, const Parser::Server& currentServ);
		static std::string	getRedirResponse(const Parser::Server& currentServ, const std::string& redir); //I still dont know what do i need here, i supose request is needed but idk if something else
		static std::string	getCgiResponse(const Parser::Location& currentLoc, Request& req, std::vector<char *>& envp, const Parser::Server& currentServ, std::string& cleanPath);
		static std::string	getAutoindexResponse(const Parser::Server& currentServ, std::string& cleanPath);
		static std::string	getFileResponse(const Parser::Location& currentLoc, const Parser::Server& currentServ, std::string& cleanPath);
		static std::string	parsePath(std::string servPath, std::string locPath, std::string reqPath);
};