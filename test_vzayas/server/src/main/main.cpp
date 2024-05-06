#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>

// this libraries are for Codespaces
#include <string.h>
#include <cstdio>

// Creating two server sockets, serverSocket1 for port 8080 and serverSocket2 for port 8081. 
// Binding each socket to its respective port and then add both sockets to the set of file descriptors that select monitors. 
// In the select loop, checking for activity on both sockets and handle connections accordingly.

int main() {
    // Create sockets for both ports
    int serverSocket1 = socket(AF_INET, SOCK_STREAM, 0);
    int serverSocket2 = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket1 == -1 || serverSocket2 == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Bind the sockets to their respective addresses and ports
    struct sockaddr_in serverAddr1, serverAddr2;
    memset(&serverAddr1, 0, sizeof(serverAddr1));
    memset(&serverAddr2, 0, sizeof(serverAddr2));
    serverAddr1.sin_family = AF_INET;
    serverAddr1.sin_addr.s_addr = INADDR_ANY;
    serverAddr1.sin_port = htons(8080);

    serverAddr2.sin_family = AF_INET;
    serverAddr2.sin_addr.s_addr = INADDR_ANY;
    serverAddr2.sin_port = htons(8081);

    if (bind(serverSocket1, (struct sockaddr *)&serverAddr1, sizeof(serverAddr1)) < 0) {
        perror("Bind failed for port 8080");
        return 1;
    }

    if (bind(serverSocket2, (struct sockaddr *)&serverAddr2, sizeof(serverAddr2)) < 0) {
        perror("Bind failed for port 8081");
        return 1;
    }

    // Listen for incoming connections on both ports
    if (listen(serverSocket1, 5) < 0) {
        perror("Listen failed for port 8080");
        return 1;
    }

    if (listen(serverSocket2, 5) < 0) {
        perror("Listen failed for port 8081");
        return 1;
    }

    std::cout << "Server started. Listening on ports 8080 and 8081..." << std::endl;

    // Initialize the set of sockets
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(serverSocket1, &readfds);
    FD_SET(serverSocket2, &readfds);

    int maxfd = std::max(serverSocket1, serverSocket2);

// --- main loop --- //
    while (true) {
        // Copy the fd set, since select modifies it
        fd_set tmpfds = readfds;

        // Use select to wait for activity on any of the sockets
        int activity = select(maxfd + 1, &tmpfds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            perror("Select error");
            return 1;
        }

        // If activity on any server socket, accept the connection
        if (FD_ISSET(serverSocket1, &tmpfds))
        {
            int clientSocket;
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            clientSocket = accept(serverSocket1, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (clientSocket < 0)
            {
                perror("Accept failed");
                return 1;
            // Handle connection on port 8080
            // Accept incoming connection, read/write data, etc.
            // Similar for serverSocket2
            }
            std::cout << "Socket1: Connection accepted from " << inet_ntoa(clientAddr.sin_addr) << ": " << ntohs(clientAddr.sin_port) << std::endl;

            // Add the new client socket to the set
            FD_SET(clientSocket, &readfds);
            if (clientSocket > maxfd)
            {
                maxfd = clientSocket;
            }
        }
        if (FD_ISSET(serverSocket2, &tmpfds))
        {
            // Handle connection on port 8081
            // Accept incoming connection, read/write data, etc.
            // Similar for serverSocket1
            int clientSocket2;
            struct sockaddr_in clientAddr2;
            socklen_t clientAddrLen2 = sizeof(clientAddr2);
            clientSocket2 = accept(serverSocket2, (struct sockaddr *)&clientAddr2, &clientAddrLen2);
            if (clientSocket2 < 0)
            {
                perror("Accept failed");
                return 1;
            }
            std::cout << "Socket2: Connection accepted from " << inet_ntoa(clientAddr2.sin_addr) << ": " << ntohs(clientAddr2.sin_port) << std::endl;

            // Add the new client socket to the set
            FD_SET(clientSocket2, &readfds);
            if (clientSocket2 > maxfd)
            {
                maxfd = clientSocket2;
            }
        }
    }
    // --- main loop --- //


    // Close the server sockets (never reached in this project)
    close(serverSocket1);
    close(serverSocket2);

    return 0;
}
