// #include <unistd.h>
// #include "Parser.hpp"
// #include "Request.hpp"
// #include "colors.h"
// #include <stdexcept>
// #include <iomanip>
// #include <iostream>
// #include <vector>
// #include <map>
// #include <algorithm>
// #include <cstring>
// #include <cstdlib>
// #include <cerrno>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <fstream>
// #include <poll.h>
// #include <sys/wait.h>

// #define BUFFER_SIZE 1024

// std::map<std::string, std::string> sessions;

// int obtainIntFromStr(std::string const & num) {
// 	int ret = 0;
// 	std::istringstream iss(num);
// 	if (!(iss >> ret)) {
// 		throw std::runtime_error("Failed to convert string to size_t in obtainSizeFromStr()");
// 	}
// 	return ret;
// }

// std::map<std::string, std::string> parse_http_request(const std::string& request) {
// 	std::map<std::string, std::string> ret;

// 	std::istringstream iss(request);
// 	std::string line;
// 	while (std::getline(iss, line)) {
// 		if (line.empty()) {
// 			break;
// 		}

// 		if (line.find("GET") != std::string::npos || line.find("POST") != std::string::npos || line.find("DELETE") != std::string::npos) {
// 			ret["request_line"] = line;
// 		}
// 		size_t pos = line.find(':');
// 		if (pos != std::string::npos) {
// 			std::string key = line.substr(0, pos);
// 			std::string value = line.substr(pos + 1);
// 			value.erase(0, value.find_first_not_of(" \t"));
// 			value.erase(value.find_last_not_of(" \t") + 1);
// 			ret[key] = value;
// 		}
// 	}
// 	return ret;
// }

// std::string generateSessionID() {
// 	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
// 	std::string sessionID;

// 	for (int i = 0; i < 16; ++i) {
// 		sessionID += alphanum[rand() % (sizeof(alphanum) - 1)];
// 	}

// 	return sessionID;
// }

// void setCookie(std::map<std::string, std::string> &headers, const std::string &name, const std::string &value) {
// 	headers["Set-Cookie"] = name + "=" + value + "; Path=/; HttpOnly";
// }

// std::string getCookie(const std::string &cookies, const std::string &name) {
// 	size_t start = cookies.find(name + "=");
// 	if (start != std::string::npos) {
// 		start += name.length() + 1;
// 		size_t end = cookies.find(';', start);
// 		if (end == std::string::npos) {
// 			end = cookies.length();
// 		}
// 		return cookies.substr(start, end - start);
// 	}
// 	return "";
// }

// void handleSession(const std::map<std::string, std::string> &requestHeaders, std::map<std::string, std::string> &responseHeaders) {
// 	std::string sessionID = getCookie(requestHeaders.at("Cookie"), "sessionID");
// 	if (sessionID.empty() || sessions.find(sessionID) == sessions.end()) {
// 		sessionID = generateSessionID();
// 		sessions[sessionID] = "new_session_data";
// 		setCookie(responseHeaders, "sessionID", sessionID);
// 	} else {
// 		std::string sessionData = sessions[sessionID];
// 		sessions[sessionID] = "updated_session_data";
// 	}
// }

// void executeCGI(int client_fd, const std::string &scriptPath, const std::string &query) {
// 	int cgiInput[2];
// 	int cgiOutput[2];

// 	if (pipe(cgiInput) < 0 || pipe(cgiOutput) < 0) {
// 		perror("pipe");
// 		return;
// 	}

// 	pid_t pid = fork();
// 	if (pid < 0) {
// 		perror("fork");
// 		return;
// 	}

// 	if (pid == 0) { // Child process
// 		dup2(cgiInput[0], STDIN_FILENO);
// 		dup2(cgiOutput[1], STDOUT_FILENO);

// 		close(cgiInput[1]);
// 		close(cgiOutput[0]);

// 		char *args[] = { (char *)scriptPath.c_str(), NULL };
// 		char *env[] = { (char *)("QUERY_STRING=" + query).c_str(), NULL };

// 		execve(scriptPath.c_str(), args, env);
// 		exit(EXIT_FAILURE);
// 	} else { // Parent process
// 		close(cgiInput[0]);
// 		close(cgiOutput[1]);

// 		write(cgiInput[1], query.c_str(), query.size());
// 		close(cgiInput[1]);

// 		char buffer[1024];
// 		int bytesRead;
// 		while ((bytesRead = read(cgiOutput[0], buffer, sizeof(buffer))) > 0) {
// 			send(client_fd, buffer, bytesRead, 0);
// 		}
// 		close(cgiOutput[0]);

// 		waitpid(pid, NULL, 0);
// 	}
// }

// void handle_connection(int client_socket, Parser const &parser) {
// 	Request requestParser;
// 	requestParser.readRequest(client_socket);

// 	std::cout << "-----**********------" RESET << std::endl;
// 	std::cout << GREEN "Request print!" RESET << std::endl;

// 	std::string port = requestParser.getPort();
// 	std::string path = requestParser.getPath();
// 	std::cout << std::setw(15) << "method: !" << requestParser.getMethod() << "!\n";
// 	std::cout << std::setw(15) << "version: !" << requestParser.getVersion() << "!\n";
// 	std::cout << std::setw(15) << "path: !" << path << "!\n";
// 	std::cout << std::setw(15) << "host: !" << requestParser.getHost() << "!\n";
// 	std::cout << std::setw(15) << "port: !" << port << "!\n";
// 	std::cout << std::setw(15) << "connection: !" << requestParser.getConection() << "!\n";

// 	std::cout << "-----**********------" RESET << std::endl;

// 	Parser::Location location = parser.getCurLocation(path, port);
// 	std::cout << YELLOW << std::setw(40) << "======>>> print test <<<======" << std::endl;
// 	std::cout << "name: " << location.name << "\nroot: " << location.root << "\nmethod: ";
// 	for (std::vector<std::string>::const_iterator it = location.methods.begin(); it != location.methods.end(); ++it)
// 		std::cout << *it << " ";
// 	std::cout << std::endl;
// 	std::cout << std::setw(40) << "======>>> print test <<<======" << "\n" RESET << std::endl;

// 	std::string request = requestParser.getPort();
// 	std::map<std::string, std::string> headers = parse_http_request(request);
// 	std::map<std::string, std::string> responseHeaders;
// 	handleSession(headers, responseHeaders);

// 	if (path.find("/cgi-bin/") == 0) {
// 		std::string scriptPath = path.substr(9);
// 		std::string query = ""; // Extract query string from URI if necessary
// 		executeCGI(client_socket, scriptPath, query);
// 	} else if (request.find("8080") != std::string::npos) {
// 		std::ifstream file("docs/index.html", std::ios::in | std::ios::binary);
// 		if (!file.is_open()) {
// 			std::cerr << "Failed to open index.html" << std::endl;
// 			return;
// 		}

// 		std::string html_content;
// 		char ch;
// 		while (file.get(ch)) {
// 			html_content += ch;
// 		}
// 		file.close();

// 		char content_length[20];
// 		sprintf(content_length, "Content-Length: %lu", html_content.size());
// 		std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
// 		for (const auto &header : responseHeaders) {
// 			response += header.first + ": " + header.second + "\r\n";
// 		}
// 		response += content_length;
// 		response += "\r\n\r\n" + html_content;
// 		std::cout << "response: " << response << std::endl;
// 		send(client_socket, response.c_str(), response.size(), 0);
// 	} else if (request.find("8081") != std::string::npos) {
// 		std::ifstream file("docs/indexcolor.html", std::ios::in | std::ios::binary);
// 		if (!file.is_open()) {
// 			std::cerr << "Failed to open indexcolor.html" << std::endl;
// 			return;
// 		}

// 		std::string html_content;
// 		char ch;
// 		while (file.get(ch)) {
// 			html_content += ch;
// 		}
// 		file.close();

// 		char content_length[20];
// 		sprintf(content_length, "Content-Length: %lu", html_content.size());
// 		std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
// 		for (const auto &header : responseHeaders) {
// 			response += header.first + ": " + header.second + "\r\n";
// 		}
// 		response += content_length;
// 		response += "\r\n\r\n" + html_content;
// 		std::cout << "response: " << response << std::endl;
// 		send(client_socket, response.c_str(), response.size(), 0);
// 	} else {
// 		std::string requested_url = "/";
// 		if (requested_url == "/") {
// 			std::ifstream file("docs/indexbird.html", std::ios::in | std::ios::binary);
// 			if (!file.is_open()) {
// 				std::cerr << "Failed to open indexbird.html" << std::endl;
// 				return;
// 			}

// 			std::string html_content;
// 			char ch;
// 			while (file.get(ch)) {
// 				html_content += ch;
// 			}
// 			file.close();

// 			char content_length[20];
// 			sprintf(content_length, "Content-Length: %lu", html_content.size());
// 			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
// 			for (const auto &header : responseHeaders) {
// 				response += header.first + ": " + header.second + "\r\n";
// 			}
// 			response += content_length;
// 			response += "\r\n\r\n" + html_content;
// 			std::cout << "response: " << response << std::endl;
// 			send(client_socket, response.c_str(), response.size(), 0);
// 		}
// 	}

// 	close(client_socket);
// }

// std::vector<int> setSockets(Parser const &parser) {
// 	std::map<std::string, Parser::Server>::const_iterator server_iter;
// 	std::vector<int> ret(parser.getServers().size());
// 	int i = 0;
// 	for (server_iter = parser.getServers().begin(); server_iter != parser.getServers().end(); ++server_iter) {
// 		const Parser::Server &server = server_iter->second;
// 		int port = obtainIntFromStr(server.port);
// 		ret[i] = socket(AF_INET, SOCK_STREAM, 0);
// 		if (ret[i] == -1) {
// 			perror("Error creating socket");
// 		}

// 		int yes = 1;
// 		if (setsockopt(ret[i], SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) == -1) {
// 			perror("Error setting socket options");
// 		}

// 		struct sockaddr_in server_addr;
// 		memset(&server_addr, 0, sizeof(server_addr));
// 		server_addr.sin_family = AF_INET;
// 		server_addr.sin_addr.s_addr = INADDR_ANY;
// 		server_addr.sin_port = htons(port);

// 		if (bind(ret[i], reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
// 			perror("Bind failed for port");
// 		}

// 		if (listen(ret[i], 5) < 0) {
// 			perror("Listen failed for port");
// 		}
// 		i++;
// 		std::cout << "Server started. Listening on port " << port << "..." << std::endl;
// 	}
// 	return ret;
// }

// void mainLoop(std::vector<struct pollfd> pollfds, std::vector<int> server_sockets, Parser const &parser) {
// 	while (true) {
// 		int activity = poll(&pollfds[0], pollfds.size(), -1);
// 		if (activity < 0) {
// 			perror("Poll error");
// 		}

// 		for (size_t i = 0; i < pollfds.size(); ++i) {
// 			if (pollfds[i].revents & POLLIN) {
// 				int client_socket;
// 				struct sockaddr_in client_addr;
// 				socklen_t client_addr_len = sizeof(client_addr);
// 				client_socket = accept(server_sockets[i], reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
// 				if (client_socket < 0) {
// 					perror("Accept failed");
// 				}
// 				std::cout << "Socket " << i + 1 << ": Connection accepted from " << inet_ntoa(client_addr.sin_addr)
// 						<< ": " << ntohs(client_addr.sin_port) << std::endl;

// 				handle_connection(client_socket, parser);
// 			}
// 		}
// 	}
// 	for (size_t i = 0; i < server_sockets.size(); ++i) {
// 		close(server_sockets[i]);
// 	}
// }

// int main(int argc, char *argv[]) {
// 	std::string config_path;
// 	if (argc == 2)
// 		config_path = argv[1];
// 	else if (argc == 1)
// 		config_path = "configurations/default.conf";
// 	else {
// 		std::cerr << "Error: too many arguments." << std::endl;
// 		return 1;
// 	}

// 	Parser parser;
// 	try {
// 		parser.parse(config_path);
// 	} catch (const std::exception & e) {
// 		std::cerr << "Parser error: " << e.what() << std::endl;
// 		return 1;
// 	}
// 	std::cout << "\n" << std::setw(40) << "======>>> print test <<<======" << std::endl;
// 	std::cout << parser << RESET;
// 	std::cout << std::setw(40) << "======>>> print test <<<======" << "\n" << std::endl;

// 	Parser::Location location = parser.getCurLocation("/cgi-bin/", "8080\0");

// 	std::cout << CYAN << std::setw(40) << "======>>> print test <<<======" << std::endl;
// 	std::cout << "name: " << location.name << "\nroot: " << location.root << "\nmethod: ";
// 	for (std::vector<std::string>::const_iterator it = location.methods.begin(); it != location.methods.end(); ++it)
// 		std::cout << *it << " ";
// 	std::cout << std::endl;
// 	std::cout << std::setw(40) << "======>>> print test <<<======" << "\n" RESET << std::endl;

// 	std::vector<int> server_sockets = setSockets(parser);

// 	std::vector<struct pollfd> pollfds(server_sockets.size());
// 	for (size_t i = 0; i < server_sockets.size(); ++i) {
// 		pollfds[i].fd = server_sockets[i];
// 		pollfds[i].events = POLLIN;
// 	}

// 	mainLoop(pollfds, server_sockets, parser);

// 	for (size_t i = 0; i < server_sockets.size(); ++i) {
// 		close(server_sockets[i]);
// 	}
// 	return 0;
// }



#include <unistd.h>
#include "Parser.hpp"
#include "Request.hpp"
#include "Utils.hpp"
#include "colors.h"
#include <stdexcept>
#include <iomanip>

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <poll.h>

#include <sstream>
#include <sys/types.h>
#include <cstdio>   // for remove

#define BUFFER_SIZE 1024

int	obtainIntFromStr(std::string const & num)
{
	int	ret = 0;
	std::istringstream iss(num);
	if (!(iss >> ret))
	{
		throw std::runtime_error("Failed to convert string to size_t in obtainSizeFromStr()");
		// std::cerr << "convert error." << std::endl;
		// return 1; //put exception, end will never get return when it returns error
	}
	return ret;
}

std::map<std::string, std::string> parse_http_request(const std::string& request)
{
	std::map<std::string, std::string> ret;

	// Split the request into lines
	std::istringstream iss(request);
	std::string line;
	while (std::getline(iss, line))
	{
		// Check if the line is empty (end of ret)
		if (line.empty())
		{
			break;
		}

		// Split each line into key-value pairs
		if (line.find("GET") != std::string::npos || line.find("POST") != std::string::npos || line.find("DELETE") != std::string::npos)
		{
			ret["request_line"] = line;
		}
		size_t pos = line.find(':');
		if (pos != std::string::npos)
		{
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			// Trim leading and trailing whitespace from value
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);
			ret[key] = value;
		}
	}

	return ret;
}

std::string getDeleteResponse(const std::string &fullPath)
{
	std::ostringstream oss;
	std::string response;
	std::string body;

	if (remove(fullPath.c_str()) == 0) {
		std::cout << CYAN "removed successfully" RESET << std::endl;
		body =
			"<html>"
			"<head><title>File Deleted</title></head>"
			"<body><h1>File Deleted</h1><p>The requested file has been deleted successfully.</p></body>"
			"</html>";
		oss << "HTTP/1.1 200 OK\r\n"
			<< "Content-Type: text/html\r\n"
			<< "Content-Length: " << body.size() << "\r\n"
			<< "\r\n" << body;
	} else {
		std::cout << RED "failed to remove file" RESET << std::endl;
		body =
			"<html>"
			"<head><title>Internal Server Error</title></head>"
			"<body><h1>Internal Server Error</h1><p>Failed to delete the requested file on the server.</p></body>"
			"</html>";
		oss << "HTTP/1.1 500 Internal Server Error\r\n"
			<< "Content-Type: text/html\r\n"
			<< "Content-Length: " << body.size() << "\r\n"
			<< "\r\n" << body;
	}

	response = oss.str();
	return response;
}

void handle_connection(int client_socket, Parser const & parser)
{

	Request requestParser;
	requestParser.readRequest(client_socket);
//TEST PRINT//
	std::cout << "-----**********------" RESET << std::endl;
	std::cout << GREEN "Request print!" RESET << std::endl;

	std::string port = requestParser.getPort();
	std::string path = requestParser.getPath();
	std::cout << std::setw(15) << "method: " << requestParser.getMethod() << "\n";
	std::cout << std::setw(15) << "version: " << requestParser.getVersion() << "\n";
	std::cout << std::setw(15) << "path: " << path << "\n";
	std::cout << std::setw(15) << "host: " << requestParser.getHost() << "\n";
	std::cout << std::setw(15) << "port: " << port << "\n";
	std::cout << std::setw(15) << "connection: " << requestParser.getConection() << "\n";

	std::cout << "-----**********------" RESET << std::endl;

	Parser::Location location;
	Parser::Server curServer = parser.getServer(port);
	//************** check if it's working the getter **************//

	// std::string fullpath = "docs" + path;
	std::string fullpath = curServer.root + path;
	if (Utils::fileExists(fullpath))
	{
		std::cout << CYAN "FILE EXISTS!!: " << fullpath << RESET << std::endl;
		if (requestParser.getMethod() == "DELETE")
		{
			std::string response = getDeleteResponse(fullpath);

			std::cout << "Response being sent: \n" << response << std::endl;
			ssize_t sent_bytes = send(client_socket, response.c_str(), response.size(), 0);
			if (sent_bytes < 0)
			{
				std::cerr << RED "Error sending response" RESET << std::endl;
			}
			else
			{
				std::cout << GREEN "Response sent successfully" RESET << std::endl;
			}

			if (close(client_socket) < 0)
			{
				std::cerr << RED "Error closing socket" RESET << std::endl;
			}
			else
			{
				std::cout << GREEN "Socket closed successfully" RESET << std::endl;
			}
			return;
		}
	}
	else
	{
		std::cout << RED "FILE DOES NOT EXISTS!!: " << fullpath << RESET << std::endl;
	}

	try
	{
		location = parser.getCurLocation(path, port);
	}
	catch (const std::exception & e)
	{
		std::cerr << "getCurLocation has failed: " << e.what() << std::endl;

		// Define and send a 404 Not Found response
		std::string not_found_response = 
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 118\r\n"
			"\r\n"
			"<html>"
			"<head><title>404 Not Found</title></head>"
			"<body><h1>404 Not Found</h1><p>The requested URL was not found on this server.</p></body>"
			"</html>";

		send(client_socket, not_found_response.c_str(), not_found_response.size(), 0);
		close(client_socket);
		return;
	}
	// Parser::Location location = parser.getCurLocation("/cgi-bin/", "8080");

	std::cout << YELLOW << std::setw(40) << "======>>> print test <<<======" << std::endl;
	std::cout << "name: " << location.name << "\nroot: " << location.root << "\nmethod: ";
	for (std::vector<std::string>::const_iterator it = location.methods.begin(); it != location.methods.end(); ++it)
		std::cout << *it << " ";
	std::cout << std::endl;
	std::cout << std::setw(40) << "======>>> print test <<<======" << "\n" RESET << std::endl;

	//************** check if it's working the getter **************//
	

		// Serve appropriate file based on the requested URL

//*************this is to know how it works HTTP request*************//
        std::string request = requestParser.getPort();
        // std::string request = buffer;
		if (request.find("8080") != std::string::npos) {
			std::ifstream file("docs/index.html", std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "Failed to open index.html" << std::endl;
				return;
			}

			std::string html_content;
			char ch;
			while (file.get(ch))
			{
				html_content += ch;
			}
			file.close();

			char content_length[20]; // Assuming content length won't exceed 20 characters
			sprintf(content_length, "Content-Length: %lu", html_content.size());
			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
			response += content_length;
			response += "\r\n\r\n" + html_content;
			std::cout << "response: " << response << std::endl;
			send(client_socket, response.c_str(), response.size(), 0);
		}    
		else if (request.find("8081") != std::string::npos) {
			std::ifstream file("docs/indexcolor.html", std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "Failed to open index.html" << std::endl;
				return;
			}

			std::string html_content;
			char ch;
			while (file.get(ch))
			{
				html_content += ch;
			}
			file.close();

			char content_length[20]; // Assuming content length won't exceed 20 characters
			sprintf(content_length, "Content-Length: %lu", html_content.size());
			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
			response += content_length;
			response += "\r\n\r\n" + html_content;
			std::cout << "response: " << response << std::endl;
			send(client_socket, response.c_str(), response.size(), 0);
		}
//*************this is to know how it works HTTP request*************//
		else
		{
		// Example:
		std::string requested_url = "/"; // Place holder for requested URL
		if (requested_url == "/") {
			std::ifstream file("docs/indexbird.html", std::ios::in | std::ios::binary);
			if (!file.is_open())
			{
				std::cerr << "Failed to open index.html" << std::endl;
				return;
			}

			std::string html_content;
			char ch;
			while (file.get(ch))
			{
				html_content += ch;
			}
			file.close();

			char content_length[20]; // Assuming content length won't exceed 20 characters
			sprintf(content_length, "Content-Length: %lu", html_content.size());
			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
			response += content_length;
			response += "\r\n\r\n" + html_content;
			std::cout << "response: " << response << std::endl;
			send(client_socket, response.c_str(), response.size(), 0);
		}
		}
	
	close(client_socket);
}




std::vector<int>	setSockets(Parser const & parser)
{
	std::map<std::string, Parser::Server>::const_iterator server_iter;
	std::vector<int> ret(parser.getServers().size());
	int	i = 0;
	for (server_iter = parser.getServers().begin(); server_iter != parser.getServers().end(); ++server_iter)
	{
		const Parser::Server &server = server_iter->second;
		int	port = obtainIntFromStr(server.port);
		ret[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (ret[i] == -1)
		{
			perror("Error creating socket");
			// return NULL;
		}

		int yes = 1;
		if (setsockopt(ret[i], SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) == -1)
		{
			perror("Error setting socket options");
			// return 1;
		}

		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(port);

		if (bind(ret[i], reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
		{
			perror("Bind failed for port");
			// return 1;
		}

		if (listen(ret[i], 5) < 0)
		{
			perror("Listen failed for port");
			// return 1;
		}
		i++;
		std::cout << "Server started. Listening on port " << port << "..." << std::endl;
	}
	return ret;
}

void	mainLoop(std::vector<struct pollfd> pollfds, std::vector<int> server_sockets, Parser const & parser)
{
	while (true)
	{
		int activity = poll(&pollfds[0], pollfds.size(), -1);
		if (activity < 0)
		{
			perror("Poll error");
			// return 1;
		}

		for (size_t i = 0; i < pollfds.size(); ++i)
		{
			if (pollfds[i].revents & POLLIN)
			{
				int client_socket;
				struct sockaddr_in client_addr;
				socklen_t client_addr_len = sizeof(client_addr);
				client_socket = accept(server_sockets[i], reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
				if (client_socket < 0)
				{
					perror("Accept failed");
					// return 1;
				}
				std::cout << "Socket " << i+1 << ": Connection accepted from " << inet_ntoa(client_addr.sin_addr)
							<< ": " << ntohs(client_addr.sin_port) << std::endl;

				handle_connection(client_socket, parser);
			}
		}
	}
	for (size_t i = 0; i < server_sockets.size(); ++i)
	{
		close(server_sockets[i]);
	}
}

int	main(int argc, char *argv[])
{
//************** config parser **************//
	std::string	config_path;
	if (argc == 2)
		config_path = argv[1];
	else if (argc == 1)
		config_path = "configurations/default.conf";
	else
	{
		std::cerr << "Error: too many arguments." << std::endl;
		return 1;
	}
	Parser parser;//construct without argv, but with webserv.conf
	// Parser parser(config_path);//constructor with config_path

	try
	{
		parser.parse(config_path);
	}
	catch (const std::exception & e)
	{
		std::cerr << "Parser error: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "\n" << std::setw(40) << "======>>> print test <<<======" << std::endl;
	std::cout << parser << RESET;
	std::cout << std::setw(40) << "======>>> print test <<<======" << "\n" << std::endl;
//************** config parser **************//

//************** check if it's working the getter **************//
Parser::Location location = parser.getCurLocation("/cgi/", "8080\0");

std::cout << CYAN << std::setw(40) << "======>>> print test <<<======" << std::endl;
std::cout << "name: " << location.name << "\nroot: " << location.root << "\nmethod: ";
for (std::vector<std::string>::const_iterator it = location.methods.begin(); it != location.methods.end(); ++it)
	std::cout << *it << " ";
std::cout << std::endl;
std::cout << std::setw(40) << "======>>> print test <<<======" << "\n" RESET << std::endl;

//************** check if it's working the getter **************//

	
	//maybe I need to put Server class outside of the parser class

//************** provide sockets **************//
	std::vector<int> server_sockets = setSockets(parser);
//************** provide sockets **************//

	std::vector<struct pollfd> pollfds(server_sockets.size());
	for (size_t i = 0; i < server_sockets.size(); ++i)
	{
		pollfds[i].fd = server_sockets[i];
		pollfds[i].events = POLLIN;
	}

	mainLoop(pollfds, server_sockets, parser);

	for (size_t i = 0; i < server_sockets.size(); ++i)
	{
		close(server_sockets[i]);
	}
	return 0;
}

// #include "WebServer.hpp"

// int main(int argc, char **argv, char **envp)
// {
// 	WebServer webserv;
// 	std::string configFile;

// 	if (argc > 2)
// 		std::cerr << "Error too many args" << std::endl;
// 	else if (argc < 2)
// 		configFile = "configurations/default.conf";
// 	else
// 		configFile = argv[1];
	
// 	if (webserv.initialize(envp, configFile))
// 		return (1);
// 	webserv.serverLoop();
// 	webserv.serverClose();
// }
