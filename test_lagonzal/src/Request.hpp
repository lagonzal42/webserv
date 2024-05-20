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

class Request{

	private:
		std::string	_method;
		std::string	_version;
		std::string	_path;
		std::string	_host;
		std::string	_port;
		
		bool	_keepAlive;

	public:
		Request(int client_socket);
		~Request(void);

		std::string	getMethod(void) const;
		std::string	getVersion(void) const;
		std::string	getPath(void) const;
		std::string	getHost(void) const;
		std::string	getPort(void) const;
		bool		getConection(void) const;
};