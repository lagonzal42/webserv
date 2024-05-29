
#pragma once

#include "Parser.hpp"
#include "Request.hpp"
#include "MimeDict.hpp"

class ResponseGenerator
{
	public:
		/*=============================== GET RESPONSES ===================================*/
		static const char	*generateGetResponse(Parser::Location& currentLoc, Request& req);
		static const char	*errorResponse(int errorCode/*, Parser::Server& server*/);
		static const char	*getRedirResponse(); //I still dont know what do i need here, i supose request is needed but idk if something else
		static const char	*getCgiResponse(Request& req);
		static const char	*getAutoindexResponse(Request& req);
		static const char	*getFileResponse(Request& req);
};