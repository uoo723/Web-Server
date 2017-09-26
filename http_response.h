#ifndef MAKE_RESPONSE_H
#define MAKE_RESPONSE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "http_request.h"

#define STATUS_LEN 50
#define MAX_HEADERS 50
#define MAX_ELEMENT_SIZE 500

typedef struct {
    char status[STATUS_LEN];
    char headers[MAX_HEADERS][2][MAX_ELEMENT_SIZE];
    int num_headers;
    int content_length;
    void *content;
} http_response_t;

void set_status(http_response_t *response, enum http_status status);
void set_header(http_response_t *response, char *field, char *value);
void make_response(http_response_t *response, http_request_t *request);
void make_response_string(http_response_t *response, char **dst, int *dst_size);
void print_http_response(http_response_t *response);

#ifdef __cplusplus
}
#endif
#endif
