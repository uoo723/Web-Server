#include "http_utils.h"

char *find_header_value(char **fields, char **values, int num, char *search) {
    int i;
    for (i = 0; i < num; i++) {
        if (strcmp(fields[i], search) == 0) {
            return values[i];
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
