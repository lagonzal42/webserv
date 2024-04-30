//This is a test to know how to run the server, using c++11 things

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

//This is the function handle_connection, which handles each incoming client connection. 
//It takes a socket file descriptor (client_socket) as input. 
//It reads data from the client socket into a buffer, prints the received data to the console, 
//constructs an HTTP response containing "Hello, World!" message, and sends the response back to the client. 
//Finally, it closes the client socket.
void handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int valread = read(client_socket, buffer, BUFFER_SIZE);
    if (valread > 0) {
        std::cout << "Received: " << buffer << std::endl;

        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1 style=\"color: green;\">Kaixo, Mundua!</h1>\n<h2>This is the first page.</h2>";
        send(client_socket, response.c_str(), response.size(), 0);
    }
    close(client_socket);
}

//This is the main() function. It declares variables for the server socket file descriptor (server_fd), 
//client socket file descriptor (client_socket), socket address structure (address), and the length of the address structure (addrlen).
//It then creates a socket using the socket() function. 
//If the socket creation fails, it prints an error message and exits the program.
int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

//These lines configure the server address (address). 
//It sets the address family to IPv4 (AF_INET), binds the socket to all available network interfaces (INADDR_ANY), 
//and sets the port number to the predefined PORT value. 
//Then it binds the socket to the address using the bind() function. 
//If the binding fails, it prints an error message and exits the program.
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
//This line makes the server start listening for incoming connections using the listen() function. 
//It specifies the maximum number of queued connections (backlog) as 3. 
//If the listen call fails, it prints an error message and exits the program. 
//Otherwise, it prints a message indicating that the server is listening on the specified port.
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << PORT << std::endl;

//This while loop accepts incoming client connections using the accept() function within an infinite loop. 
//When a new client connection arrives, it spawns a new thread to handle the connection using the handle_connection function. 
//The detach() function is called on the thread object, allowing the thread to run independently. 
//The server continues to accept new connections indefinitely.
    while (true) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::thread(handle_connection, client_socket).detach();
    }

    return 0;
}
