#include <stdio.h>
#include "http_request.h"

void print_http_request(http_request_t *request) {
    int i;
    printf("path: %s\n", request->path);
    printf("method: %s\n", http_method_str(request->method));
    for (i = 0; i < request->num_headers; i++) {
        printf("%s: %s\n", request->headers[i][0], request->headers[i][1]);
    }

    printf("body: \n%s\n", request->body);
    fflush(stdout);
}

int on_url_cb(http_parser *parser, const char *at, size_t len) {
    if (!parser->data) return 0;

    http_request_t *request = (http_request_t *) parser->data;
    struct http_parser_url u;
    int is_connect = parser->method == HTTP_CONNECT;

    http_parser_url_init(&u);
    if (http_parser_parse_url(at, len, is_connect, &u) == 0) {
        strncpy(request->path, at + u.field_data[UF_PATH].off,
            u.field_data[UF_PATH].len);
    }

    return 0;
}

int on_header_field_cb(http_parser *parser, const char *at, size_t len) {
    if (!parser->data) return 0;

    http_request_t *request = (http_request_t *) parser->data;

    if (request->last_header_element != FIELD) {
        request->num_headers++;
    }

    strncat(request->headers[request->num_headers - 1][0], at, len);
    request->last_header_element = FIELD;

    return 0;
}

int on_header_value_cb(http_parser *parser, const char *at, size_t len) {
    if (!parser->data) return 0;

    http_request_t *request = (http_request_t *) parser->data;

    strncat(request->headers[request->num_headers - 1][1], at, len);
    request->last_header_element = VALUE;

    return 0;
}

int on_body_cb(http_parser *parser, const char *at, size_t len) {
    if (!parser->data) return 0;

    http_request_t *request = (http_request_t *) parser->data;
    strncat(request->body, at, len);

    return 0;
}

int on_message_complete_cb(http_parser *parser) {
    if (!parser->data) return 0;

    http_request_t *request = (http_request_t *) parser->data;
    request->method = parser->method;

    return 0;
}
