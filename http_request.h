#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "http_parser.h"

#define MAX_HEADERS 50
#define MAX_ELEMENT_SIZE 500

typedef struct {
    char path[MAX_ELEMENT_SIZE];
    enum { NONE=0, FIELD, VALUE } last_header_element;
    char headers[MAX_HEADERS][2][MAX_ELEMENT_SIZE];
    int num_headers;
    char body[MAX_ELEMENT_SIZE];
    int method;
} http_request_t;

void print_http_request(http_request_t *request);
int on_url_cb(http_parser *parser, const char *at, size_t len);
int on_header_field_cb(http_parser *parser, const char *at, size_t len);
int on_header_value_cb(http_parser *parser, const char *at, size_t len);
int on_body_cb(http_parser *parser, const char *at, size_t len);
// int on_message_begin_cb(http_parser *parser);
int on_message_complete_cb(http_parser *parser);
// int on_headers_complete_cb(http_parser *parser);

#ifdef __cplusplus
}
#endif
#endif
