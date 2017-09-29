#include <stdio.h>
#include <stdlib.h>
#include "http_common.h"

void set_header(http_headers_t *headers, char *field, char *value) {
    int i;
    for (i = 0; i < headers->num_headers; i++) {
        if (strcmp(headers->field[i], field) == 0) {
            memset(headers->value[i], 0, MAX_ELEMENT_SIZE);
            strcpy(headers->value[i], value);
            return;
        }
    }

    strcpy(headers->field[headers->num_headers], field);
    strcpy(headers->value[headers->num_headers], value);
    headers->num_headers++;
}

char *find_header_value(http_headers_t *headers, char *search) {
    int i;
    for (i = 0; i < headers->num_headers; i++) {
        if (strcmp(headers->field[i], search) == 0) {
            return headers->value[i];
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
