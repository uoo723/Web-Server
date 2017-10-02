#ifndef HTTP_COMMON_H
#define HTTP_COMMON_H
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HEADERS 50
#define MAX_ELEMENT_SIZE 500
#define MAX_RANGE 50

typedef struct {
    char field[MAX_HEADERS][MAX_ELEMENT_SIZE];
    char value[MAX_HEADERS][MAX_ELEMENT_SIZE];
    int num_headers;
} http_headers_t;

typedef struct {
    enum { UNIT_NONE=0, BYTES } unit;
    int start[MAX_RANGE];
    int end[MAX_RANGE];
    int num_range;
} range_t;

void set_header(http_headers_t *headers, char *field, char *value);
char *find_header_value(http_headers_t *headers, char *search);
int get_range(char *str, range_t *range);
void print_range(range_t *range);

#ifdef __cplusplus
}
#endif
#endif
