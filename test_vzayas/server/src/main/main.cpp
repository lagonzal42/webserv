#include "../../inc/webserver.h"

void	doGet(std::string& fileName, int client_socket)
{
	std::cout << "-----Doing Get-----" << std::endl;
	std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);

	// if (!file.is_open())
    // {
    //     if (fileName != "favicon.ico") {
    //         std::cerr << "Failed to open " << fileName << std::endl;
    //     }
    //     return;
    // }

	// Read the file content
	std::string html_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	//substitution to the to_string method
	std::stringstream ss;
    ss << html_content.size();

	// Create a simple response
	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + ss.str() + "\r\n\r\n" + html_content;
	std::cout << "response: " << response << std::endl;
	send(client_socket, response.c_str(), response.size(), 0);
	close(client_socket);
}

void doPost(std::string fileName, std::string request, int client_socket, bool chunked)
{
	std::cerr << "-----Doing Post-----" << std::endl;

	// Parse the request to get the boundary string, filename, and file content
    std::istringstream ss(request);
    std::string line;
    std::string boundary;
    std::string fileContent;

	std::cout << "Request: " << request << std::endl;

	if (chunked) {
		parseChunked(request);
	}

    // Get the boundary string
    while (std::getline(ss, line) && line != "\r") {
		std::cerr << "Line: " << line << std::endl;
        size_t pos = line.find("boundary=");
        if (pos != std::string::npos) {
            boundary = "--" + line.substr(pos + 9);
			boundary.erase(boundary.rfind("\r"));
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
			if (fileName.empty()) {
				fileName = "unknown";
			}
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
        std::cout << "File Content: " << fileContent << std::endl;
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
	int yes = 1;

	setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));
	
	if (valread > 0) {
		// std::cout << "Received: " << buffer << std::endl;

		std::string req(buffer);
		std::string method;
		std::istringstream reqStream(req);

		bool isChunked = false;
		if (req.find("Transfer-Encoding: chunked") != std::string::npos) {
			isChunked = true;
			std::cerr << "Chunked request" << std::endl;
		}

		std::getline(reqStream, method, ' ');
		
		// std::cout << "Method: " << method << std::endl;

		std::string fileName;
		std::getline(reqStream, fileName, ' ');

		// std::cout << "Requested file: " << fileName << std::endl;

		if (method == "POST") {
            // Procesar la solicitud POST
                doPost(fileName, req, client_socket, isChunked);
        }
		else {
			// Procesar la solicitud GET
			if (fileName.length() == 1)
				fileName = "/home/vzayas/webserv/test_vzayas/admin/web/index.html";
			doGet(fileName, client_socket);
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
		unsigned int port = 8080 + i;
		serverAddrs[i].sin_port = htons(port);
		serverAddrs[i].sin_addr.s_addr = INADDR_ANY; //binds to any address available(?)

		if (bind(serverSocket, (struct sockaddr *) &serverAddrs[i], sizeof(serverAddrs[i])) < 0)
		{
			std::cerr << "Bind failed in port " << port + i;
			perror(" ");
			return (1);
		}
		if (listen(serverSocket, 5) < 0)
		{
			std::cerr <<  "Failed listen on port " << port + i;
			perror(" ");
			return (1);
		}

   		std::cout << "Listening on port " << port + i << std::endl;

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
