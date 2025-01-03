#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "request.h"
#include "config.h"

// Networks
#include <arpa/inet.h>
#include <sys/socket.h>

#define HEADERS_404 "HTTP/1.1 404 Not Found\r\n"
#define HEADERS_TEXT_HTML "HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/html; charset=utf-8\r\n\
"

void parse_http_header(char *buffer, struct HttpHeader *header)
{
    // Seperate the header and value, example:
    // Set-Cookie: value
    sscanf(buffer, "%99[^:]: %99[^\n]", header->name, header->value);
}

void parse_http_request(char *buffer, struct HttpRequest *request)
{
    char *line = strtok(buffer, "\r\n");

    // Parse request line
    sscanf(line, "%s %s %s", request->method, request->url, request->version);

    // Parse headers
    request->header_count = 0;
    while ((line = strtok(NULL, "\r\n")) != NULL && line[0] != '\0')
    {
        if (request->header_count < MAX_HEADERS)
        {
            // Parse Header
            size_t header_size = MAX_HEADER_NAME_LENGTH + MAX_HEADER_VALUE_LENGTH;
            char header[header_size];
            strncpy(header, line, header_size - 1);
            header[header_size - 1] = '\0';

            // Turn it to a Header Struct
            parse_http_header(header, &request->headers[request->header_count]);

            request->header_count++;
        }
    }

    // Find and set body
    request->body = strstr(buffer, "\r\n\r\n");
    if (request->body)
    {
        request->body += 4; // Move past the \r\n\r\n
        request->body_length = strlen(request->body);
    }
    else
    {
        request->body = NULL;
        request->body_length = 0;
    }
}

char *get_requested_file(struct HttpRequest *request, const char *directory)
{
    // Ensure the URL length does not exceed MAX_URL_LENGTH
    size_t url_length = strnlen(request->url, MAX_URL_LENGTH);
    size_t directory_length = strlen(directory);

    size_t total_length = url_length + directory_length + 2; // +2 for '\0' and /

    char *relative_path = malloc(total_length);
    if (relative_path == NULL)
    {
        perror("get_path");
        return NULL;
    }

    // Set the prefix to directory
    strncpy(relative_path, directory, directory_length);
    relative_path[directory_length] = '/';

    // Copy the URL into relative_path starting from index 1
    strncpy(relative_path + directory_length, request->url, url_length);

    relative_path[total_length - 1] = '\0';
    return relative_path;
}

