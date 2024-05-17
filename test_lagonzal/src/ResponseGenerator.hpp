/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseGenerator.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagonzal <larraingonzalez@gmail.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/16 13:34:12 by lagonzal          #+#    #+#             */
/*   Updated: 2024/05/16 13:34:12 by lagonzal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>

#define OK 100
#define NOT_FOUND 404
#define FORBIDEN 403
#define SERVER_ERROR 500

#ifndef BUFFERSIZE
# define BUFFERSIZE 1024
#endif

class ResponseGenerator
{
	private:
		ResponseGenerator();
		ResponseGenerator(const ResponseGenerator&);
		const ResponseGenerator& operator=(const ResponseGenerator& p);

	public:
		~ResponseGenerator();
		
        static const char	*generateResponse(int responseCode);
		static const char	*generateResponse(int responseCode, int pipeFD);
};
