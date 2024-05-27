#include "../../inc/webserver.h"

extern char ** environ;

void	doCgi(std::string& fileName, int client_socket)
{
	int pipes[2];


	if (pipe(pipes) != 0)
	{
		std::cout << "Failed to create pipes" << std::endl;
		return ;
	}

	int id = fork();

	if (id == 0)
	{
		dup2(pipes[1], STDOUT_FILENO);
		close(pipes[1]);
		close(pipes[0]);
		fileName = "." + fileName;
		char *filepath[] = {const_cast<char *>(fileName.c_str()), NULL};

		if (execve(filepath[0], filepath, environ) == -1)
		{
			std::cout << "execve failed" << std::endl;
			close(client_socket);
		}
	}
	else
	{
		int status;
		close(pipes[1]);
		
		waitpid(id, &status, 0);
		if (WIFEXITED(status)) //proccess terminated normally by no signal
		{
			if (WEXITSTATUS(status) != 0)
			{
				std::string response = "HTTP/1.1 404 KO\r\nContent-Type: text/html\r\nContent-Length: 0 \r\n\r\n";
				send(client_socket, response.c_str(), response.size(), 0);
			}
		}
		char buffer[BUFFER_SIZE];
		int readed = read(pipes[0], buffer, BUFFER_SIZE);

		close(pipes[0]);
		std::string content(buffer);
		if (readed > 0)
		{
			std::cout << "Response: " << content;

			std::stringstream ss;
    		ss << content.size();

			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + content;
			send(client_socket, response.c_str(), response.size(), 0);
		}
		close(client_socket);
	}
}

void	doStandard(std::string& fileName, int client_socket)
{
	
	std::cout << "Doing standard" << std::endl;
	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Failed to open index.html" << std::endl; // this trigger all time, returning the page or not. ???
		return;
	}

	std::string html_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	//substitution to the to_string method
	std::stringstream ss;
    ss << html_content.size();

	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + html_content;
	std::cout << "response: " << response << std::endl;
	send(client_socket, response.c_str(), response.size(), 0);
	close(client_socket);
}

// Una función para obtener el valor de una cabecera
std::string getHeaderValue(const std::string& headers, const std::string& headerName) {
    std::regex regex(headerName + ": (.+)\r\n");
    std::smatch match;
    if (std::regex_search(headers, match, regex)) {
        return match[1].str();
    }
    return "";
}

void doPost(std::string& fileName, std::string& request, int client_socket) {
    // Encuentra el inicio de las cabeceras y el cuerpo de la solicitud
    size_t header_pos = request.find("\r\n\r\n");
    if (header_pos == std::string::npos) {
        std::cerr << "Invalid POST request" << std::endl;
        return;
    }
    std::string headers = request.substr(0, header_pos);
    std::string body = request.substr(header_pos + 4);

    // Verifica el tipo de contenido
    std::string content_type = getHeaderValue(headers, "Content-Type");
    if (content_type.find("multipart/form-data") == std::string::npos) {
        std::cerr << "Unsupported Content-Type" << std::endl;
        return;
    }

    // Extrae el límite de los datos
    std::string boundary = "--" + content_type.substr(content_type.find("boundary=") + 9);
    
    // Procesa las partes del multipart
    size_t start = 0;
    while ((start = body.find(boundary, start)) != std::string::npos) {
        size_t end = body.find(boundary, start + boundary.size());
        if (end == std::string::npos) break;

        std::string part = body.substr(start + boundary.size() + 2, end - start - boundary.size() - 4);
        start = end;

        // Extrae las cabeceras de la parte
        size_t part_header_pos = part.find("\r\n\r\n");
        if (part_header_pos == std::string::npos) continue;
        std::string part_headers = part.substr(0, part_header_pos);
        std::string part_body = part.substr(part_header_pos + 4);

        // Obtiene el nombre del archivo
        std::regex regex("Content-Disposition: form-data; name=\"([^\"]+)\"; filename=\"([^\"]+)\"");
        std::smatch match;
        if (std::regex_search(part_headers, match, regex)) {
            std::string field_name = match[1].str();
            std::string filename = match[2].str();

            // Guarda el archivo
            std::ofstream ofs("./uploads/" + filename, std::ios::binary);
            ofs << part_body;
            ofs.close();

            std::cout << "Saved file: " << filename << std::endl;
        }
    }

    // Crear una respuesta simple
    std::string response_body = "<html><body><h1>Files Uploaded</h1></body></html>";
    std::stringstream ss;
    ss << response_body.size();
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + response_body;
    send(client_socket, response.c_str(), response.size(), 0);

    close(client_socket);
}

/**
 * @brief Checks the request minimally and if the requested file is in the cgi-bin directory executes it and send the result, otherwise it does the standard
 * 
 * @param client_socket 
 */
void handle_connection(int client_socket) 
{
	char buffer[BUFFER_SIZE] = {0};
	int valread = read(client_socket, buffer, BUFFER_SIZE);
	if (valread > 0) {
		std::cout << "Received: " << buffer << std::endl;

		std::string req(buffer);
		std::string method;
		std::istringstream reqStream(req);

		std::getline(reqStream, method, ' ');
		
		std::cout << "Method: " << method << std::endl;

		std::string fileName;
		std::getline(reqStream, fileName, ' ');

		std::cout << "Requested file: " << fileName << std::endl;

		if (method == "POST") {
            // Procesar la solicitud POST
            if (fileName == "/home/vzayas-s/Documents/webserv/test_vzayas/admin/web/index.html")
                doPost(fileName, req, client_socket);
            else if (fileName.find("test_vzayas/server/http/uploads/cgi") != std::string::npos)
                doCgi(fileName, client_socket);
            else
                doStandard(fileName, client_socket);
        }
		else {
			if (fileName.find("test_vzayas/server/http/uploads/cgi") != std::string::npos) {
				std::cout << "The file is in the cgi-bin directory" << std::endl;
				doCgi(fileName, client_socket);
			}
			else {
				std::cout << "The file is not in the cgi-bin directory" << std::endl;
				if (fileName.length() == 1)
					fileName = "/home/vzayas-s/Documents/webserv/test_vzayas/admin/web/index.html";
				doStandard(fileName, client_socket);
			}
		}
	}
	close(client_socket);
}

/**
 * @brief This function will be divided in the future.
 * 
 * 
 * @return int 
 */
int main(void)
{
	/*
		This first part initializes the sockets especified by SOCK_N, binds them and puts them to listen.
		The config parse should go before this, and the signal handling too.
	*/
	std::vector<int> serverSockets;
	std::vector<pollfd> fds;
	std::vector<sockaddr_in> serverAddrs(SOCK_N);

	for (size_t i = 0; i < SOCK_N; i++)
	{
		int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

		if (serverSocket == -1)
		{
			perror("Server socket creation error");
			return (1);
		}

		memset(&serverAddrs[i], 0, sizeof(serverAddrs[i]));
		serverAddrs[i].sin_family = AF_INET;
		serverAddrs[i].sin_port = htons(8080 + i);
		serverAddrs[i].sin_addr.s_addr = INADDR_ANY; //binds to any address available(?)

		if (bind(serverSocket, (struct sockaddr *) &serverAddrs[i], sizeof(serverAddrs[i])) < 0)
		{
			std::cerr << "Bind failed in port" << 8080 + i;
			perror("");
			return (1);
		}
		if (listen(serverSocket, 5) < 0)
		{
			std::cerr <<  "Failed listen on port " << 8080 + i;
			perror("");
			return (1);
		}

   		std::cout << "Listening on port " << 8080 + i << std::endl;

		//is a pollfd structure vector. 
		fds.push_back({serverSocket, POLLIN, 0});
		//is a vector for the listening sockets.
		serverSockets.push_back(serverSocket);
	}

	//This is the server loop
	while (true)
	{
		//executes poll looking for events
		int events = poll(&fds[0], fds.size(), 5000);
		
		if (events != 0)
		{
			//iterates through the pollfds
			for (size_t i = 0; i < fds.size(); i++)
			{
				if (fds[i].revents & POLLIN) //POLLIN flag means that the socket is ready. revents is the event that is returned by poll inside the pollfd structure
				{
					std::vector<int>::iterator it = std::find(serverSockets.begin(), serverSockets.end(), fds[i].fd);
					if (it != serverSockets.end()) //Creates a client socket if the socket with the in event was one of the servers ones.
					{
						struct sockaddr_in clientAddr;
						socklen_t clientAddrLen = sizeof(clientAddr);
						int clientSocket = accept(*it , (struct sockaddr *)&clientAddr, &clientAddrLen);
						fds.push_back({clientSocket, POLLIN, 0});
					}
					else //Uses the client socket and closes it
					{
						handle_connection(fds[i].fd);
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						i--;
					}
				}
			} // socket event check loop end
		}
		else
			std::cout << "No events happened" << std::endl;
	} //server loop end
}
