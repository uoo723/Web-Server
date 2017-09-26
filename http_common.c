#include "http_common.h"

char *find_header_value(char **fields, char **values, int num, char *search) {
    int i;
    for (i = 0; i < num; i++) {
        if (strcmp(fields[i], search) == 0) {
            return values[i];
        }
    }

    return NULL;
}
