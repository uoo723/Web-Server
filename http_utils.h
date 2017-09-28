#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "http_request.h"

char *find_header_value(http_request_t *request, char *search);
void get_ranges(char *str, int *start, int *end);

#ifdef __cplusplus
}
#endif
#endif
