#pragma once

#include <string>

class Request{

	private:
		std::string	_method;
		std::string	_version;
		std::string	_path;
		std::string	_host;
		std::string	_port;
		std::string	_encoding;
		
		bool	_keepAlive;

	public:
		Request(void);
		~Request(void);

		int readRequest(int client_socket);
		std::string	getMethod(void) const;
		std::string	getVersion(void) const;
		std::string	getPath(void) const;
		std::string	getHost(void) const;
		std::string	getPort(void) const;
		std::string	getEncoding(void) const;
		bool		getConection(void) const;
		std::string	extractNumbers(std::string const & str);
};