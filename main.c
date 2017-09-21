#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define BACKLOG 10

int main(int argc, char *argv[]) {
    int socketfd, new_socket, valread;
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    int server_addrlen = sizeof(server_addr);
    int client_addrlen;
    char buffer[BUFFER_SIZE] = {0};
    char *message;

    // Create ipv4 TCP socket
    if ((socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket option to be reuse address to avoid error "Address already in use"
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt,
        (socklen_t) sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    // Required on Linux >= 3.9
    #ifdef SO_REUSEPORT
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &opt,
        (socklen_t) sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEPORT) failed");
        exit(EXIT_FAILURE);
    }
    #endif

    // Set IP socket address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the address to the socket
    if (bind(socketfd, (struct sockaddr *) &server_addr,
        (socklen_t) sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections on the socket
    if (listen(socketfd, BACKLOG) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Accept a connection on the socket
        if ((new_socket = accept(socketfd, (struct sockaddr *) &client_addr,
            (socklen_t *) &client_addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Read data from the socket connected via accept()
        memset(&buffer, 0, BUFFER_SIZE);
        if (recv(new_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        printf("%s\n", buffer);

        // Write data to the socket connected via accept()
        message =
        "HTTP/1.1 200 OK\r\n"
        "Date: Thu, 19 Feb 2009 12:27:04 GMT\r\n"
        "Server: Apache/2.2.3\r\n"
        "Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\r\n"
        "ETag: \"56d-9989200-1132c580\"\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 18\r\n"
        "Accept-Ranges: bytes\r\n"
        "Connection: close\n"
        "\r\n"
        "I got your message";

        if (send(new_socket, message, strlen(message), 0) < 0) {
            perror("write failed");
            exit(EXIT_FAILURE);
        }

        close(new_socket);
    }
    close(socketfd);
}
