#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "make_response.h"

#define MAX_HEADERS 50
#define MAX_ELEMENT_SIZE 500

typedef struct {
    char status[50];
    char body[100*1024*1024]; // Up to 100 MB
    int num_headers;
    char headers[MAX_HEADERS][2][MAX_ELEMENT_SIZE];
    int content_length;
} http_response_t;

static void set_header(http_response_t *response, char *field, char *value) {
    strcpy(response->headers[response->num_headers][0], field);
    strcpy(response->headers[response->num_headers][1], value);
    response->num_headers++;
}

static char *get_mime_type(char *path) {
    char *tmp = strrchr(path, '.');
    char *ext = tmp ? tmp + 1 : NULL;

    if (!ext) {
        return "text/plain";
    }

    if (strcmp(ext, "html") == 0) {
        return "text/html";
    }

    if (strcmp(ext, "css") == 0) {
        return "text/css";
    }

    if (strcmp(ext, "png") == 0) {
        return "image/png";
    }

    if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
        return "image/jpeg";
    }

    if (strcmp(ext, "mp4") == 0) {
        return "video/mp4";
    }

    return"text/plain";
}

static void get_current_time(char *buf, size_t len) {
    time_t now = time(NULL);
    struct tm tm = *gmtime(&now);
    strftime(buf, len, "%a, %d %b %Y %H:%M:%S %Z", &tm);
}

static void set_status(http_response_t *response, enum http_status status) {
    switch (status) {
    case HTTP_STATUS_CONTINUE:
        strcpy(response->status, "100 Continue");
        break;

    case HTTP_STATUS_SWITCHING_PROTOCOLS:
        strcpy(response->status, "101 Switching Protocols");
        break;

    case HTTP_STATUS_PROCESSING:
        strcpy(response->status, "102 Switching Protocols");
        break;

    case HTTP_STATUS_OK:
        strcpy(response->status, "200 OK");
        break;

    case HTTP_STATUS_CREATED:
        strcpy(response->status, "201 Created");
        break;

    case HTTP_STATUS_ACCEPTED:
        strcpy(response->status, "202 Accepted");
        break;

    case HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION:
        strcpy(response->status, "203 Non-Authoritative Information");
        break;

    case HTTP_STATUS_NO_CONTENT:
        strcpy(response->status, "204 No Content");
        break;

    case HTTP_STATUS_RESET_CONTENT:
        strcpy(response->status, "205 Reset Content");
        break;

    case HTTP_STATUS_PARTIAL_CONTENT:
        strcpy(response->status, "206 Partial Content");
        break;

    case HTTP_STATUS_MULTI_STATUS:
        strcpy(response->status, "207 Multi-Status");
        break;

    case HTTP_STATUS_ALREADY_REPORTED:
        strcpy(response->status, "208 Already Reported");
        break;

    case HTTP_STATUS_IM_USED:
        strcpy(response->status, "226 IM Used");
        break;

    case HTTP_STATUS_MULTIPLE_CHOICES:
        strcpy(response->status, "300 Multiple Choices");
        break;

    case HTTP_STATUS_MOVED_PERMANENTLY:
        strcpy(response->status, "301 Moved Permanently");
        break;

    case HTTP_STATUS_FOUND:
        strcpy(response->status, "302 Found");
        break;

    case HTTP_STATUS_SEE_OTHER:
        strcpy(response->status, "303 See Other");
        break;

    case HTTP_STATUS_NOT_MODIFIED:
        strcpy(response->status, "304 Not Modified");
        break;

    case HTTP_STATUS_USE_PROXY:
        strcpy(response->status, "305 Use Proxy");
        break;

    case HTTP_STATUS_TEMPORARY_REDIRECT:
        strcpy(response->status, "307 Temporary Redirect");
        break;

    case HTTP_STATUS_PERMANENT_REDIRECT:
        strcpy(response->status, "308 Permanent Redirect");
        break;

    case HTTP_STATUS_BAD_REQUEST:
        strcpy(response->status, "400 Bad Request");
        break;

    case HTTP_STATUS_UNAUTHORIZED:
        strcpy(response->status, "401 Unauthorized");
        break;

    case HTTP_STATUS_PAYMENT_REQUIRED:
        strcpy(response->status, "402 Payment Required");
        break;

    case HTTP_STATUS_FORBIDDEN:
        strcpy(response->status, "403 Forbidden");
        break;

    case HTTP_STATUS_NOT_FOUND:
        strcpy(response->status, "404 Not Found");
        break;

    case HTTP_STATUS_METHOD_NOT_ALLOWED:
        strcpy(response->status, "405 Method Not Allowed");
        break;

    case HTTP_STATUS_NOT_ACCEPTABLE:
        strcpy(response->status, "406 Not Acceptable");
        break;

    case HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED:
        strcpy(response->status, "407 Proxy Authentication Required");
        break;

    case HTTP_STATUS_REQUEST_TIMEOUT:
        strcpy(response->status, "408 Request Timeout");
        break;

    case HTTP_STATUS_CONFLICT:
        strcpy(response->status, "409 Conflict");
        break;

    case HTTP_STATUS_GONE:
        strcpy(response->status, "410 Gone");
        break;

    case HTTP_STATUS_LENGTH_REQUIRED:
        strcpy(response->status, "411 Length Required");
        break;

    case HTTP_STATUS_PRECONDITION_FAILED:
        strcpy(response->status, "412 Precondition Failed");
        break;

    case HTTP_STATUS_PAYLOAD_TOO_LARGE:
        strcpy(response->status, "413 Payload Too Large");
        break;

    case HTTP_STATUS_URI_TOO_LONG:
        strcpy(response->status, "414 URI Too Long");
        break;

    case HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE:
        strcpy(response->status, "415 Unsupported Media Type");
        break;

    case HTTP_STATUS_RANGE_NOT_SATISFIABLE:
        strcpy(response->status, "416 Range Not Satisfiable");
        break;

    case HTTP_STATUS_EXPECTATION_FAILED:
        strcpy(response->status, "417 Expectation Failed");
        break;

    case HTTP_STATUS_MISDIRECTED_REQUEST:
        strcpy(response->status, "421 Misdirected Request");
        break;

    case HTTP_STATUS_UNPROCESSABLE_ENTITY:
        strcpy(response->status, "422 Unprocessable Entity");
        break;

    case HTTP_STATUS_LOCKED:
        strcpy(response->status, "423 Locked");
        break;

    case HTTP_STATUS_FAILED_DEPENDENCY:
        strcpy(response->status, "424 Failed Dependency");
        break;

    case HTTP_STATUS_UPGRADE_REQUIRED:
        strcpy(response->status, "426 Upgrade Required");
        break;

    case HTTP_STATUS_PRECONDITION_REQUIRED:
        strcpy(response->status, "428 Precondition Required");
        break;

    case HTTP_STATUS_TOO_MANY_REQUESTS:
        strcpy(response->status, "429 Too Many Requests");
        break;

    case HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE:
        strcpy(response->status, "431 Request Header Fields Too Large");
        break;

    case HTTP_STATUS_UNAVAILABLE_FOR_LEGAL_REASONS:
        strcpy(response->status, "451 Unavailable For Legal Reasons");
        break;

    case HTTP_STATUS_INTERNAL_SERVER_ERROR:
        strcpy(response->status, "500 Internal Server Error");
        break;

    case HTTP_STATUS_NOT_IMPLEMENTED:
        strcpy(response->status, "501 Not Implemented");
        break;

    case HTTP_STATUS_BAD_GATEWAY:
        strcpy(response->status, "502 Bad Gateway");
        break;

    case HTTP_STATUS_SERVICE_UNAVAILABLE:
        strcpy(response->status, "503 Service Unavailable");
        break;

    case HTTP_STATUS_GATEWAY_TIMEOUT:
        strcpy(response->status, "504 Gateway Timeout");
        break;

    case HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED:
        strcpy(response->status, "505 HTTP Version Not Supported");
        break;

    case HTTP_STATUS_VARIANT_ALSO_NEGOTIATES:
        strcpy(response->status, "506 Variant Also Negotiates");
        break;

    case HTTP_STATUS_INSUFFICIENT_STORAGE:
        strcpy(response->status, "507 Insufficient Storage");
        break;

    case HTTP_STATUS_LOOP_DETECTED:
        strcpy(response->status, "508 Loop Detected");
        break;

    case HTTP_STATUS_NOT_EXTENDED:
        strcpy(response->status, "510 Not Extended");
        break;

    case HTTP_STATUS_NETWORK_AUTHENTICATION_REQUIRED:
        strcpy(response->status, "511 Network Authentication Required");
        break;
    }
}

static void make_response_string(char **dst, int *dst_size,
    http_response_t *response) {

    size_t size = sizeof(char) * (80 * 1024 + 100 * 1024 * 1024);
    size_t content_offset = 0;
    char *version = "HTTP/1.1 ";
    int i;

    *dst = malloc(size);
    memset(*dst, 0, size);

    strcat(*dst, version);
    content_offset += strlen(version);
    strcat(*dst, response->status);
    content_offset += strlen(response->status);
    strcat(*dst, "\r\n");
    content_offset += 2;

    for (i = 0; i < response->num_headers; i++) {
        strcat(*dst, response->headers[i][0]);
        content_offset += strlen(response->headers[i][0]);
        strcat(*dst, ": ");
        content_offset += 2;
        strcat(*dst, response->headers[i][1]);
        content_offset += strlen(response->headers[i][1]);
        strcat(*dst, "\r\n");
        content_offset += 2;
    }
    strcat(*dst, "\r\n");
    content_offset += 2;
    memcpy(*dst + content_offset, response->body, response->content_length);
    *dst_size = content_offset + response->content_length;
}

void make_response(char **dst, int *dst_size, http_request_t *request) {
    // printf("make_response\n");
    http_response_t *response = malloc(sizeof(http_response_t));
    char path[100] = "html";
    int content_length;
    int is_server_error = 0;
    char time_string[500] = {0};
    memset(response, 0, sizeof(http_response_t));

    set_status(response, HTTP_STATUS_OK);
    get_current_time(time_string, 500);
    set_header(response, "Date", time_string);
    set_header(response, "Connection", "close");
    set_header(response, "Server", "C Server/0.1");

    if (request->method != HTTP_GET) {
        set_status(response, HTTP_STATUS_METHOD_NOT_ALLOWED);
        make_response_string(dst, dst_size, response);
        return;
    }

    if (strcmp(request->path, "/") == 0) {
        strcat(path, "/index.html");
    } else {
        strcat(path, request->path);
    }

    FILE *fp = fopen(path, "rb");

    if (!fp) {
        if (errno == ENOENT) {
            fp = fopen("html/404.html", "rb");
            set_status(response, HTTP_STATUS_NOT_FOUND);

            if (!fp) {
                set_status(response, HTTP_STATUS_INTERNAL_SERVER_ERROR);
                is_server_error = 1;
            }
        } else {
            set_status(response, HTTP_STATUS_INTERNAL_SERVER_ERROR);
            is_server_error = 1;
        }
    }

    if (!is_server_error) {
        fseek(fp, 0, SEEK_END);
        response->content_length = content_length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(response->body, sizeof(char), content_length, fp);
        fclose(fp);

        char str_content_len[100] = {0};
        sprintf(str_content_len, "%d", content_length);
        set_header(response, "Content-Length", str_content_len);
        set_header(response, "Content-Type", get_mime_type(path));
    }

    make_response_string(dst, dst_size, response);
    // printf("dst_size: %d\n", *dst_size);
    // printf("%s\n", *dst);
    free(response);
}
