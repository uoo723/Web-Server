#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#include "http_parser.h"
#include "http_request.h"
#include "http_response.h"

#define BUFFER_SIZE (80*1024)
#define PORT 8080
#define BACKLOG 10

static void sigpipe_handler(int signum) { /* Do nothing */ }

static void error(char *str) {
    perror(str);
    exit(EXIT_FAILURE);
}

static void recv_request(int socketfd, http_parser *parser) {
    int nparsed, recved;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    while ((recved = recv(socketfd, buffer, BUFFER_SIZE, 0)) >= 0) {
        // if ()
    }
}

int main(int argc, char *argv[]) {
    signal(SIGPIPE, sigpipe_handler);

    int socketfd;
    int opt;
    int sock_buf_size;
    socklen_t opt_size = (socklen_t) sizeof(opt);

    // Create ipv4 TCP socket
    if ((socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        error("socket failed");
    }

    opt = 1;
    // Set socket option to be reuse address to avoid error "Address already in use"
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, opt_size) < 0) {
        error("setsockopt(SO_REUSEADDR) failed");
    }

    // Required on Linux >= 3.9
    #ifdef SO_REUSEPORT
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &opt, opt_size) < 0) {
        error("setsockopt(SO_REUSEPORT) failed");
    }
    #endif

    if (getsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, &opt, &opt_size) < 0) {
        error("getsockopt(SOL_SNDBUF) failed");
    }
    sock_buf_size = opt;

    struct sockaddr_in server_addr;
    // Set IP socket address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the address to the socket
    if (bind(socketfd, (struct sockaddr *) &server_addr,
        sizeof(server_addr)) < 0) {
        error("bind failed");
    }

    // Listen for connections on the socket
    if (listen(socketfd, BACKLOG) < 0) {
        error("listen failed");
    }

    int nparsed, recved;
    http_parser *parser = malloc(sizeof(http_parser));
    http_parser_settings settings;
    http_request_t *request = malloc(sizeof(http_request_t));

    http_parser_settings_init(&settings);

    settings.on_url = on_url_cb;
    settings.on_header_field = on_header_field_cb;
    settings.on_header_value = on_header_value_cb;
    settings.on_body = on_body_cb;
    settings.on_message_complete = on_message_complete_cb;

    http_parser_init(parser, HTTP_REQUEST);
    parser->data = request;

    char buffer[BUFFER_SIZE];
    char *message;

    while (1) {
        int new_socket;
        struct sockaddr client_addr;
        socklen_t client_addrlen;

        // Accept a connection on the socket
        if ((new_socket = accept(socketfd, (struct sockaddr *) &client_addr,
            &client_addrlen)) < 0) {
            error("accept failed");
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

        // print_http_request((http_request_t *) parser->data);

        int msg_size;
        FILE *fp = NULL;
        // make_response(&message, &msg_size, &fp, (http_request_t *) parser->data);
        // printf("msg_size: %d\n", msg_size);
        // printf("%s\n", message);
        if (send(new_socket, message, msg_size, 0) < 0) {
            perror("send failed");
            exit(EXIT_FAILURE);
        }

        free(message);

        if (fp) {
            char *buffer = malloc(sock_buf_size);
            int read;
            while ((read = fread(buffer, 1, sock_buf_size, fp)) > 0) {
                if (send(new_socket, buffer, read, 0) < 0) {
                    if (errno != EPIPE) {
                        perror("send file failed");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            if (ferror(fp)) {
                fprintf(stderr, "fp error occurred\n");
            }
            free(buffer);
            fclose(fp);
        }

        close(new_socket);
    }

    free(request);
    close(socketfd);

    return 0;
}
