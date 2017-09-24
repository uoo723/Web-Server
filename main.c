#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "http_parser.h"
#include "http_request.h"

#define BUFFER_SIZE (80*1024)
#define PORT 8080
#define BACKLOG 10

/*
static char *make_response(custom_data_t *data) {
    char *path = data->request_path;
    char new_path[50] = "html";
    char buf[500];

    if (strcmp(path, "/") == 0) {
        strcpy(new_path, "html/index.html");
    } else {
        strcat(new_path, path);
    }

    printf("%s\n", new_path);
    FILE *fp = fopen(new_path, "r");

    if (!fp) {
        if (errno == ENOENT) {
            fp = fopen("html/404.html", "r");
            if (!fp) {
                perror("File opending failed");
                exit(EXIT_FAILURE);
            }
        } else {
            perror("File opending failed");
            exit(EXIT_FAILURE);
        }
    }

    size_t len = fread(buf, sizeof(char), 500, fp);
    buf[len] = '\0';
    // printf("%s\n", buf);
    // int c;
    // while ((c = fgetc(fp)) != EOF) {
    //     putchar(c);
    // }
    //
    // if (ferror(fp)) {
    //     puts("I/O error when reading");
    // } else if (feof(fp)) {
    //     puts("End of file reached successfully");
    // }

    fclose(fp);

    char *format =
    "HTTP/1.1 200 OK\r\n"
    "Date: Thu, 19 Feb 2009 12:27:04 GMT\r\n"
    "Server: Apache/2.2.3\r\n"
    "Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\r\n"
    "ETag: \"56d-9989200-1132c580\"\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %zu\r\n"
    "Accept-Ranges: bytes\r\n"
    "Connection: close\n"
    "\r\n"
    "%s";

    char *new_message;
    asprintf(&new_message, format, len, buf);
    return new_message;
}
*/
int main(int argc, char *argv[]) {
    int socketfd, new_socket;
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    int client_addrlen;
    char buffer[BUFFER_SIZE];
    char *message;

    int nparsed, recved;
    http_parser *parser = malloc(sizeof(http_parser));
    http_parser_settings settings;
    http_request_t *request = malloc(sizeof(http_request_t));

    http_parser_settings_init(&settings);

    settings.on_url = on_url_cb;
    settings.on_header_field = on_header_field_cb;
    settings.on_header_value = on_header_value_cb;
    settings.on_body = on_body_cb;

    http_parser_init(parser, HTTP_REQUEST);
    parser->data = request;

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
        memset(buffer, 0, BUFFER_SIZE);

        recved = recv(new_socket, buffer, BUFFER_SIZE, 0);
        if (recved < 0) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        memset(request, 0, sizeof(http_request_t));

        nparsed = http_parser_execute(parser, &settings, buffer, recved);

        if (parser->upgrade) {
            /* handle new protocol */
        } else if (nparsed != recved) {
            /* Handle error. Usually just close the connection. */
            fprintf(stderr, "nparsed != reved");
            exit(EXIT_FAILURE);
        }

        print_http_request((http_request_t *) parser->data);

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

    free(request);
    close(socketfd);

    return 0;
}
