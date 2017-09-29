#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "http_parser.h"
#include "http_request.h"
#include "http_response.h"
#include "http_common.h"
#include "thpool.h"

#define BUFFER_SIZE (80*1024)
#define PORT 8080
#define BACKLOG 10
#define THREAD 4

typedef threadpool threadpool_t;

static void sigpipe_handler(int signum) { /* Do nothing */ }

static void error(char *str) {
    perror(str);
    exit(EXIT_FAILURE);
}

static void recv_request(int sockfd, http_parser *parser,
    http_parser_settings *settings, http_response_t *response) {
    http_request_t *request;
    int nparsed, recved;
    int total_recved = 0;
    char buf[BUFFER_SIZE];

    while (1) {
        if ((recved = recv(sockfd, buf, BUFFER_SIZE, 0)) < 0) {
            if (errno == EPIPE) {
                break;
            } else if (errno == EPROTOTYPE) {
                continue;
            } else {
                error("recv failed");
            }
        }

        total_recved += recved;

        if (total_recved > BUFFER_SIZE) {
            // TODO: 413 Payload Too Large
            break;
        }

        nparsed = http_parser_execute(parser, settings, buf, recved);
        if (parser->upgrade) { /* Do nothing */ }
        else if (nparsed != recved) {
            fprintf(stderr, "nparsed != reved");
            exit(EXIT_FAILURE);
        }

        request = (http_request_t *) parser->data;
        // print_http_request(request);
        if (request->on_message_completed) {
            break;
        }
    }
}

static void send_response(int sockfd, http_response_t *response,
    http_request_t *request) {
    char *buf;
    int buf_size;
    socklen_t opt_size = sizeof(int);
    int socket_error = 0;

    if (getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, &opt_size) < 0) {
        error("getsockopt(SO_SNDBUF) failed");
    }

    print_http_request(request);
    char *value = find_header_value(&request->headers, "Range");
    // printf("Range: %s\n", value);
    // get_range(value, NULL);
    buf = malloc(buf_size);
    memset(buf, 0, buf_size);

    make_response(response, request);

    int dst_size;
    char *dst = &buf[0];
    make_response_string(response, &dst, &dst_size);

    if (send(sockfd, buf, dst_size, 0) < 0) {
        if (errno == EPIPE || errno == EPROTOTYPE) {
            socket_error = 1;
        } else {
            error("send failed");
        }
    }

    FILE *fp = (FILE *) response->content;

    if (socket_error) {
        if (fp) fclose(fp);
        return;
    }

    if (fp) {
        int read = 0;
        memset(buf, 0, buf_size);
        while ((read = fread(buf, 1, buf_size, fp)) >= 0) {
            // printf("read: %d\n", read);
            // printf("buf_size: %d\n", buf_size);
            if (read == 0) break;
            // printf("send\n");
            // print_http_request(request);
            if (send(sockfd, buf, read, 0) < 0) {
                if (errno == EPIPE) {
                    break;
                } else if (errno == EPROTOTYPE) {
                    continue;
                } else {
                    error("send content failed");
                }
            }
            // printf("send end\n");
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

static void thread_main(void *data) {
    int *sockfd = (int *) data;
    http_parser *parser = malloc(sizeof(http_parser));
    http_request_t *request = malloc(sizeof(http_request_t));
    http_response_t *response = malloc(sizeof(http_response_t));
    http_parser_settings settings;

    // printf("sockfd: %d %p, in %u\n", *sockfd, sockfd, (unsigned int) pthread_self());

    http_parser_settings_init(&settings);
    settings.on_url = on_url_cb;
    settings.on_header_field = on_header_field_cb;
    settings.on_header_value = on_header_value_cb;
    settings.on_body = on_body_cb;
    settings.on_message_complete = on_message_complete_cb;

    http_parser_init(parser, HTTP_REQUEST);
    parser->data = request;

    memset(request, 0, sizeof(http_request_t));
    memset(response, 0, sizeof(http_response_t));

    recv_request(*sockfd, parser, &settings, response);
    send_response(*sockfd, response, request);

    free(parser);
    free(request);
    free(response);
    close(*sockfd);
    free(sockfd);
}

int main(int argc, char *argv[]) {
    signal(SIGPIPE, sigpipe_handler);

    int sockfd;
    int opt;
    int sock_buf_size;
    socklen_t opt_size = sizeof(opt);

    threadpool_t thpool;
    // Create thread pool
    if((thpool = thpool_init(THREAD)) == NULL) {
        error("thpool failed");
    }

    // Create ipv4 TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        error("socket failed");
    }

    opt = 1;
    // Set socket option to be reuse address to avoid error "Address already in use"
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, opt_size) < 0) {
        error("setsockopt(SO_REUSEADDR) failed");
    }

    // Required on Linux >= 3.9
    #ifdef SO_REUSEPORT
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, opt_size) < 0) {
        error("setsockopt(SO_REUSEPORT) failed");
    }
    #endif

    if (getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &opt, &opt_size) < 0) {
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
    if (bind(sockfd, (struct sockaddr *) &server_addr,
        sizeof(server_addr)) < 0) {
        error("bind failed");
    }

    // Listen for connections on the socket
    if (listen(sockfd, BACKLOG) < 0) {
        error("listen failed");
    }

    while (1) {
        int *new_socket = malloc(sizeof(int));
        struct sockaddr client_addr;
        socklen_t client_addrlen;

        // Accept a connection on the socket
        if ((*new_socket = accept(sockfd, (struct sockaddr *) &client_addr,
            &client_addrlen)) < 0) {
            error("accept failed");
        }

        thpool_add_work(thpool, (void *) thread_main, (void *) new_socket);
    }

    close(sockfd);
    thpool_destroy(thpool);

    return 0;
}
