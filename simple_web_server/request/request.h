#ifndef REQUEST
#define REQUEST

#include "config.h"
#include <stdlib.h>

struct HttpHeader
{
    char name[MAX_HEADER_NAME_LENGTH];
    char value[MAX_HEADER_VALUE_LENGTH];
};

struct HttpRequest
{
    char method[MAX_METHOD_LENGTH];
    char url[MAX_URL_LENGTH];
    char version[MAX_VERSION_LENGTH];
    struct HttpHeader headers[MAX_HEADERS];
    int header_count;
    char *body;
    size_t body_length;
};

void parse_http_request(char *buffer, struct HttpRequest *request);

/**
 * Gets the relative path from the HTTP Request
 * 
 * Caution: must call free()
 * 
 */
char *get_requested_file(struct HttpRequest *request, const char *directory);

void send_message(int socket, const char *message);


void send_404_page(int socket);

/**
 * Sending documents to be rendered as html or text
 */
void send_text_html(int socket, const char *text);

#endif