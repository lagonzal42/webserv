//This is a test to know how to work CGI, using c++11 things
//Execute ./a.out and connect localhost:8080/cgi-bin/

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

constexpr int BUFFER_SIZE = 1024;
constexpr int PORT = 8080;

// Function to handle CGI requests
void handle_cgi_request(int client_socket, const std::string& request) {
    // Extract CGI parameters from request (e.g., query string or POST data)
    std::string query_string;
    // Find query string if present
    size_t query_start = request.find("?");
    if (query_start != std::string::npos) {
        query_string = request.substr(query_start + 1);
    }

    // Process CGI parameters and generate dynamic content
    std::string response_body = "<h1 style=\"color: green; font-family: Papyrus;\">CGI Response</h1>\n";
    response_body += "<p style=\"color: gray; font-family: Papyrus;\">Query String: " + query_string + "</p>\n";

    // Construct HTTP response
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + std::to_string(response_body.size()) + "\r\n";
    response += "\r\n"; // End of headers
    response += response_body;

    // Send response to client
    send(client_socket, response.c_str(), response.size(), 0);
}

// Function to handle incoming client connections
void handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int valread = read(client_socket, buffer, BUFFER_SIZE);
    if (valread > 0) {
        std::string request(buffer, valread);
        std::cout << "Received: " << request << std::endl;

        // Check if it's a CGI request
        bool is_cgi_request = request.find("GET /cgi-bin/") != std::string::npos;

        if (is_cgi_request) {
            // Handle CGI request
            handle_cgi_request(client_socket, request);
        } else {
            // Handle regular request
            std::string response_body = "<h1 style=\"color: ForestGreen; background-color: red; font-family: Papyrus;\">Kaixo Mundua!</h1>\n";
            std::string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/html\r\n";
            response += "Content-Length: " + std::to_string(response_body.size()) + "\r\n";
            response += "\r\n"; // End of headers
            response += response_body;

            // Send response to client
            send(client_socket, response.c_str(), response.size(), 0);
        }
    }
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::thread(handle_connection, client_socket).detach();
    }

    return 0;
}
