#include "http_utils.h"

char *find_header_value(http_request_t *request, char *search) {
    int i;
    for (i = 0; i < request->num_headers; i++) {
        if (strcmp(request->headers[i][0], search) == 0) {
            return request->headers[i][1];
        }
    }

    return NULL;
}

void get_ranges(char *str, int *start, int *end) {
    // int i;
    // char start_str[20] = {0};
    // char end_str[20] = {0};
    //
    // *start = 0;
    // *end = -1;
    //
    // for (i = 0; i < strlen(str); i++) {
    //
    // }
}
