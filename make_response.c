#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "make_response.h"

static char *mime_type(char *path) {
    char *tmp = strrchr(path, '.');
    char *ext = tmp ? tmp + 1 : NULL;

    if (!ext) return "text/plain";

    if (strcmp(ext, "html") == 0) {
        return "text/html";
    } else if (strcmp(ext, "css") == 0) {
        return "text/css";
    } else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg")) {
        return "image/jpeg";
    }

    return "text/plain";
}

void make_response(char **dst, http_request_t *request) {
    printf("make_response\n");
    char path[100] = "html";
    char body[80*1024] = {0};
    int len;
    char *status = "200 OK";
    if (strcmp(request->path, "/") == 0) {
        strcat(path, "/index.html");
    } else {
        strcat(path, request->path);
    }

    FILE *fp = fopen(path, "rb");

    if (!fp) {
        if (errno == ENOENT) {
            fp = fopen("html/404.html", "rb");
            status = "404 Not Found";
            if (!fp) {
                perror("File opening failed");
                exit(EXIT_FAILURE);
            }
        } else {
            perror("File opending failed");
            exit(EXIT_FAILURE);
        }
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fread(body, sizeof(char), len, fp);
    body[len] = '\0';

    fclose(fp);

    char *format =
    "HTTP/1.1 %s\r\n"
    "Date: Thu, 19 Feb 2009 12:27:04 GMT\r\n"
    "Server: Apache/2.2.3\r\n"
    "Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\r\n"
    "ETag: \"56d-9989200-1132c580\"\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %zu\r\n"
    "Accept-Ranges: bytes\r\n"
    "Connection: close\n"
    "\r\n"
    "%s";

    asprintf(dst, format, status, mime_type(path), strlen(body), body);
}
