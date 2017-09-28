#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "http_request.h"

#define MAX_RANGE 50

typedef struct {
    enum { NONE=0, BYTES } unit;
    int range[MAX_RANGE][2];
    int num_range;
} range_t;

char *find_header_value(http_request_t *request, char *search);
int get_range(char *str, range_t *range);

#ifdef __cplusplus
}
#endif
#endif
