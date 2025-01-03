#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "request.h"
#include "config.h"

// Networks
#include <arpa/inet.h>
#include <sys/socket.h>

#define HEADERS_404 "HTTP/1.1 404 Not Found\r\n"
#define HEADERS_HTML "HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/html; charset=utf-8\r\n\
"
#define HEADERS_TEXT "HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/plain; charset=utf-8\r\n\
"


void send_message(int socket, const char *message)
{
    size_t length = strlen(message);
    ssize_t bytes_sent = send(socket, message, length, 0);

    if (bytes_sent == -1)
    {
        perror("send");
    }
    else if ((size_t)bytes_sent < length)
    {
        fprintf(stderr, "Partial send\n");
    }
}

void send_404_page(int socket)
{
    send_message(socket, HEADERS_404);
}

void send_html(int socket, const char *html)
{
    char response_buffer[MAX_RESPONSE_LENGTH];

    size_t content_length = strlen(html);

    snprintf(response_buffer, sizeof(response_buffer),
             "%sContent-Length: %lu\r\n\r\n%s", HEADERS_HTML,
             content_length, html); // Construct response

    printf("Responding web page:\n%s\n", response_buffer);
    send_message(socket, response_buffer);
}

void send_text(int socket, const char *text)
{
    char response_buffer[MAX_RESPONSE_LENGTH];

    size_t content_length = strlen(text);

    snprintf(response_buffer, sizeof(response_buffer),
             "%sContent-Length: %lu\r\n\r\n\%s",
             HEADERS_TEXT, content_length, text); // Construct response

    printf("Responding web page:\n%s\n", response_buffer);
    send_message(socket, response_buffer);
}
