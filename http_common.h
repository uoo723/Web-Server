#ifndef HTTP_COMMON_H
#define HTTP_COMMON_H
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

char *find_header_value(char **fields, char **values, int num, char *search);

#ifdef __cplusplus
}
#endif
#endif
