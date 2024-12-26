#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "request.h"
#include "config.h"

void parse_http_header(char *buffer, struct HttpHeader *header)
{
    sscanf(buffer, "%s: %s", header->name, header->value);
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
