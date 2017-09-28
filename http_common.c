#include <stdio.h>
#include <stdlib.h>
#include "http_common.h"

char *find_header_value(http_request_t *request, char *search) {
    int i;
    for (i = 0; i < request->num_headers; i++) {
        if (strcmp(request->headers[i][0], search) == 0) {
            return request->headers[i][1];
        }
    }

    return NULL;
}

int get_range(char *str, range_t *range) {
    char *unit = strtok(str, "=");
    if (unit == NULL) {
        return -1;
    }

    if (strcmp(unit, "bytes") == 0) {
        range->unit = BYTES;
    } else {
        range->unit = NONE;
    }

    char *ranges = strtok(NULL, "=");

    if (ranges == NULL) {
        return -1;
    }

    char *range_str = strtok(ranges, ", ");

    if (range_str == NULL) {
        char *ranges_str = strtok(ranges, "");
    }

    return 0;
}
