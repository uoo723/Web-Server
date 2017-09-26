#ifndef MAKE_RESPONSE_H
#define MAKE_RESPONSE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "http_request.h"

#define MAX_HEADERS 50
#define MAX_ELEMENT_SIZE 500
#define BUFFER_SIZE (80*1024)

typedef struct {
    char status[50];
    // char body[100*1024*1024]; // Up to 100 MB
    int num_headers;
    char headers[MAX_HEADERS][2][MAX_ELEMENT_SIZE];
    int content_length;
} http_response_t;

void make_response(char **dst, int *dst_size, FILE **fp, http_request_t *request);
void print_http_response(http_response_t *response);

#ifdef __cplusplus
}
#endif
#endif
