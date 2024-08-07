/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagonzal <larraingonzalez@gmail.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/20 12:45:17 by lagonzal          #+#    #+#             */
/*   Updated: 2024/05/20 12:45:17 by lagonzal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>



class Request{

	private:
		std::string	_method;
		std::string	_version;
		std::string	_path;
		std::string	_host;
		std::string	_port;
		std::string	_encoding;
		std::string _queryString;
		std::string _body;
		size_t		_contentLength;
		bool	_keepAlive;

	public:
		Request(void);
		~Request(void);
		Request(const Request& param);

		int readRequest(int client_socket);
		std::string	getMethod(void) const;
		std::string	getQueryString(void) const;
		std::string	getVersion(void) const;
		std::string	getPath(void) const;
		std::string	getHost(void) const;
		std::string	getPort(void) const;
		std::string	getEncoding(void) const;
		std::string	getBody(void) const;
		bool		getConection(void) const;
		size_t		getContentLength(void) const;

		void		setKeepAlive(bool pKeepAlive);

		void		print(void) const;
		std::string	extractNumbers(std::string const & str);
		void		clear(void);
		bool		empty(void);
};