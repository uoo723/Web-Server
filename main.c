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
#include "http_utils.h"

#define BUFFER_SIZE (80*1024)
#define PORT 8080
#define BACKLOG 10

static void sigpipe_handler(int signum) { /* Do nothing */ }

static void error(char *str) {
    perror(str);
    exit(EXIT_FAILURE);
}

static void recv_request(int socketfd, http_parser *parser,
    http_parser_settings *settings, http_response_t *response) {
    int nparsed, recved;
    char buffer[BUFFER_SIZE] = {0};
    int buf_size;
    // int total = 0;
    socklen_t opt_size = sizeof(buf_size);

    memset(buffer, 0, BUFFER_SIZE);

    if (getsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, &buf_size, &opt_size) < 0) {
        error("getsockopt(SO_RCVBUF) failed");
    }

    if ((recved = recv(socketfd, buffer, BUFFER_SIZE, 0)) < 0) {
        if (errno != EPIPE) {
            error("recv failed");
        }
    }

    nparsed = http_parser_execute(parser, settings, buffer, recved);
    if (parser->upgrade) { /* Do nothing */ }
    else if (nparsed != recved) {
        fprintf(stderr, "nparsed != reved");
        exit(EXIT_FAILURE);
    }

    // while ((recved = recv(socketfd, buffer, BUFFER_SIZE, 0)) >= 0) {
    //     printf("recved: %d\n", recved);
    //     if (recved == 0) break;
    //
    //     nparsed = http_parser_execute(parser, settings, buffer, recved);
    //     if (parser->upgrade) { /* Do nothing */ }
    //     else if (nparsed != recved) {
    //         fprintf(stderr, "nparsed != reved");
    //         exit(EXIT_FAILURE);
    //     }
    //
    //     total += nparsed;
    //
    //     if (total > BUFFER_SIZE) {
    //         response->status = HTTP_STATUS_PAYLOAD_TOO_LARGE;
    //         break;
    //     }
    // }
    //
    // if (recved < 0) {
    //     if (errno != EPIPE) {
    //         error("recv failed");
    //     }
    // }
}

static void send_response(int socketfd, http_response_t *response,
    http_request_t *request) {
    char buffer[BUFFER_SIZE] = {0};
    int buf_size;
    socklen_t opt_size = sizeof(buf_size);

    memset(buffer, 0, BUFFER_SIZE);

    if (getsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, &buf_size, &opt_size) < 0) {
        error("getsockopt(SO_SNDBUF) failed");
    }

    make_response(response, request);

    int dst_size;
    char *dst = &buffer[0];
    make_response_string(response, &dst, &dst_size);

    if (send(socketfd, buffer, dst_size, 0) < 0) {
        if (errno != EPIPE) {
            error("send failed");
        }
    }

    FILE *fp = (FILE *) response->content;

    if (fp) {
        int read = 0;
        while ((read = fread(buffer, 1, BUFFER_SIZE, fp)) >= 0) {
            if (read == 0) break;
            if (send(socketfd, buffer, read, 0) < 0) {
                if (errno != EPIPE) {
                    error("send file failed");
                }
            }
        }

        if (read < 0) {
            error("fread failed");
        }

        if (ferror(fp)) {
            fprintf(stderr, "ferror(fp)\n");
        }
        fclose(fp);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGPIPE, sigpipe_handler);

    int socketfd;
    int opt;
    int sock_buf_size;
    socklen_t opt_size = sizeof(opt);

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

    http_parser *parser = malloc(sizeof(http_parser));
    http_parser_settings settings;
    http_request_t *request = malloc(sizeof(http_request_t));
    http_response_t *response = malloc(sizeof(http_response_t));

    http_parser_settings_init(&settings);

    settings.on_url = on_url_cb;
    settings.on_header_field = on_header_field_cb;
    settings.on_header_value = on_header_value_cb;
    settings.on_body = on_body_cb;
    settings.on_message_complete = on_message_complete_cb;

    http_parser_init(parser, HTTP_REQUEST);
    parser->data = request;

    while (1) {
        int new_socket;
        struct sockaddr client_addr;
        socklen_t client_addrlen;

        // Accept a connection on the socket
        if ((new_socket = accept(socketfd, (struct sockaddr *) &client_addr,
            &client_addrlen)) < 0) {
            error("accept failed");
        }

        memset(request, 0, sizeof(http_request_t));
        memset(response, 0, sizeof(http_response_t));

        // printf("enter recv_request\n");
        recv_request(new_socket, parser, &settings, response);
        // printf("exit recv_request\n");
        // fflush(stdout);

        // print_http_request((http_request_t *) parser->data);

        // printf("enter send_response\n");
        send_response(new_socket, response, request);
        // printf("exit send_response\n");
        // fflush(stdout);

        close(new_socket);
    }

    free(request);
    free(response);
    close(socketfd);

    return 0;
}
