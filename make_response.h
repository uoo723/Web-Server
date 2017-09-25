#ifndef MAKE_RESPONSE_H
#define MAKE_RESPONSE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "http_request.h"

void make_response(char **dst, http_request_t *request);

#ifdef __cplusplus
}
#endif
#endif
