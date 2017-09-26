#ifndef MAKE_RESPONSE_H
#define MAKE_RESPONSE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "http_request.h"
#include "http_parser.h"

#define MAX_HEADERS 50
#define MAX_ELEMENT_SIZE 500

typedef struct {
    enum http_status status;
    char headers[MAX_HEADERS][2][MAX_ELEMENT_SIZE];
    int num_headers;
    int content_length;
    void *content;
} http_response_t;

char *get_status_string(enum http_status status);
void set_header(http_response_t *response, char *field, char *value);
void make_response(http_response_t *response, http_request_t *request);
void make_response_string(http_response_t *response, char **dst, int *dst_size);
void print_http_response(http_response_t *response);

#ifdef __cplusplus
}
#endif
#endif
