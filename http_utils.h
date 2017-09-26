#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

char *find_header_value(char **fields, char **values, int num, char *search);
void get_ranges(char *str, int *start, int *end);

#ifdef __cplusplus
}
#endif
#endif
