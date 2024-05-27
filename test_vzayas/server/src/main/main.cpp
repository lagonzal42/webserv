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
	std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
	std::cout << "DUPLICATED" << std::endl;
	if (!file.is_open())
	{
		std::cerr << "Failed to open index.html" << std::endl; // this trigger all time, returning the page or not. ???
		return;
	}

	std::cout << "DUPLICATED 2" << std::endl;
	std::string html_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	std::cout << "DUPLICATED 4" << std::endl;

	//substitution to the to_string method
	std::stringstream ss;
    ss << html_content.size();

	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + html_content;
	std::cout << "response: " << response << std::endl;
	send(client_socket, response.c_str(), response.size(), 0);
	close(client_socket);
}

// Helper function to extract filename from Content-Disposition header
std::string getFilename(const std::string &contentDisposition) {
    size_t pos = contentDisposition.find("filename=");
    if (pos == std::string::npos) {
        return "";
    }
    size_t start = contentDisposition.find('"', pos) + 1;
    size_t end = contentDisposition.find('"', start);
    return contentDisposition.substr(start, end - start);
}

void doPost(std::string fileName, std::string request, int client_socket) {
    std::cout << "Manolitpo pies de plata " << fileName << std::endl;

    std::istringstream ss(request);
    std::string line;
    std::string boundary;
    std::string fileContent;

    // Get the boundary string
    while (std::getline(ss, line) && line != "\r") {
        size_t pos = line.find("boundary=");
        if (pos != std::string::npos) {
            boundary = "--" + line.substr(pos + 9);
            break;
        }
    }

    std::cout << "Boundary: " << boundary << std::endl;

    bool isFilePart = false;

    // Read through the request to find the part headers and file content
    while (std::getline(ss, line)) {
        if (line.find(boundary) != std::string::npos) {
            isFilePart = false;
            continue;
        }
        if (line.find("Content-Disposition: ") != std::string::npos) {
            fileName = getFilename(line);
            std::cout << "Filename: " << fileName << std::endl;
        } else if (line.find("Content-Type: ") != std::string::npos) {
            isFilePart = true;
            std::getline(ss, line); // Skip the empty line after headers
        } else if (isFilePart) {
            fileContent += line + "\n";
        }
    }

    // Remove the last boundary and newline character from fileContent
    if (!fileContent.empty()) {
        size_t pos = fileContent.rfind(boundary);
        if (pos != std::string::npos) {
            fileContent.erase(pos);
        }
        if (*fileContent.rbegin() == '\n') {
            fileContent.erase(fileContent.size() - 1);
        }
    }

    std::cout << "File content length: " << fileContent.size() << std::endl;

    // Save the file to the server
    if (!fileName.empty() && !fileContent.empty()) {
        std::ofstream ofs(("/home/vzayas/webserv/test_vzayas/server/http/uploads/" + fileName).c_str(), std::ios::binary);
        if (ofs.is_open()) {
            ofs.write(fileContent.c_str(), fileContent.size());
            ofs.close();
            std::cout << "Saved file: " << fileName << std::endl;
        }
        else {
            std::cerr << "Failed to open file for writing: " << fileName << std::endl;
        }
    }
    else {
        std::cerr << "Filename or file content is empty" << std::endl;
    }

    // Create a simple response
    std::string response_body = "<html><body><h1>File Uploaded</h1></body></html>";
    std::ostringstream response_stream;
    response_stream << "HTTP/1.1 200 OK\r\n";
    response_stream << "Content-Type: text/html\r\n";
    response_stream << "Content-Length: " << response_body.size() << "\r\n";
    response_stream << "\r\n";
    response_stream << response_body;

    // Send the response to the client
    std::string response = response_stream.str();
    send(client_socket, response.c_str(), response.size(), 0);
}

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
		// while (std::getline(reqStream, fileName, ' ')) {
		// 	std::cout << "Requested file: " << fileName << std::endl;
		// }

		if (method == "POST") {
            // Procesar la solicitud POST
            if (fileName == "/home/vzayas/webserv/test_vzayas/server/http/uploads")
			{
				std::cout << "POST request de Aingeru" << std::endl;
                doPost(fileName, req, client_socket);
			}
            // else if (fileName.find("test_vzayas/server/http/uploads/cgi") != std::string::npos)
            //     doCgi(fileName, client_socket);
            // else
            //     doStandard(fileName, client_socket);
        }
		else {
			if (fileName.find("test_vzayas/server/http/uploads/cgi") != std::string::npos) {
				std::cout << "The file is in the cgi-bin directory" << std::endl;
				doCgi(fileName, client_socket);
			}
			else {
				std::cout << "The file is not in the cgi-bin directory" << std::endl;
				if (fileName.length() == 1)
					fileName = "/home/vzayas/webserv/test_vzayas/admin/web/index.html";
				doStandard(fileName, client_socket);
			}
		}
	}
	close(client_socket);
}

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
			std::cerr << "Bind failed in port " << 8080 + i;
			perror(" ");
			return (1);
		}
		if (listen(serverSocket, 5) < 0)
		{
			std::cerr <<  "Failed listen on port " << 8080 + i;
			perror(" ");
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
