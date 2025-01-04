#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "request.h"
#include "file_explorer.h"
#include "config.h"

// Networks
#include <arpa/inet.h>
#include <sys/socket.h>

#define HEADERS_404 "HTTP/1.1 404 Not Found\r\n"
#define HEADERS_HTML "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
#define HEADERS_TEXT "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n"
#define HEADERS_DOWNLOAD "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Disposition: attachment; filename=\"%s\"\r\n"
#define HEADERS_PNG "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Disposition: inline; filename=\"%s\"\r\n"

void send_404_page(int socket)
{
    send(socket, HEADERS_404, sizeof(HEADERS_404), 0);
}

void send_http_response(int socket, const char *headers, const char *content, size_t content_length) {
    // Prepare response buffer for headers
    size_t max_size = MAX_HEADERS_SIZE + content_length + 1; // +1 for null terminator
    char *response_buffer = malloc(max_size);

    if (response_buffer == NULL) {
        perror("send_http_response malloc");
        return;
    }

    // Construct headers
    int header_length = snprintf(response_buffer, max_size,
             "%sContent-Length: %lu\r\n\r\n", // Note: No %s for content here
             headers, content_length);

    // Check if header construction was successful
    if (header_length < 0 || header_length >= (int)max_size) {
        perror("snprintf failed");
        free(response_buffer);
        return;
    }

    // Send headers first
    ssize_t bytes_sent = send(socket, response_buffer, header_length, 0);
    if (bytes_sent == -1) {
        perror("send headers");
        free(response_buffer);
        return;
    }

    // Send binary content directly
    bytes_sent = send(socket, content, content_length, 0);
    if (bytes_sent == -1) {
        perror("send content");
        free(response_buffer);
        return;
    }

    free(response_buffer); // Free allocated memory
}


void send_html(int socket, const char *html) {
    size_t content_length = strlen(html);
    send_http_response(socket, HEADERS_HTML, html, content_length);
}

void send_text(int socket, const char *text) {
    size_t content_length = strlen(text);
    send_http_response(socket, HEADERS_TEXT, text, content_length);
}

void send_png(int socket, const char *filename, struct FileContent content) {
    char headers[MAX_HEADERS_SIZE];

    snprintf(headers, sizeof(headers),
             HEADERS_PNG,
             filename); // Construct headers

    // Send HTTP response with headers and binary content
    send_http_response(socket, headers, content.bytes, content.size);
}

void send_download(int socket, const char *filename, struct FileContent content) {
    size_t content_length = content.size;
    char headers[MAX_HEADERS_SIZE];

    snprintf(headers, sizeof(headers),
             HEADERS_DOWNLOAD,
             filename); // Construct headers

    send_http_response(socket, headers, content.bytes, content_length);
}

void send_directory_page(int socket, const struct FilesList *files) {
    const char *html_template_start = "<!DOCTYPE html>\n<html>\n<head>\n<title>Directory Listing</title>\n</head>\n<body>\n<h1>Directory Listing</h1>\n<ul>\n";
    const char *html_template_end = "</ul>\n</body>\n</html>";

    // Calculate total length for the full HTML response
    size_t total_length = strlen(html_template_start) + strlen(html_template_end);
    
    // Calculate length of file names for dynamic allocation
    for (size_t i = 0; i < files->length; i++) {
        total_length += strlen(files->file_names[i]) + 80; // 80 for <li><a href="...">...</a></li>
    }

    // Allocate memory for the full HTML
    char *html = malloc(total_length + 1); // +1 for null terminator
    if (html == NULL) {
        perror("malloc");
        return;
    }

    // Start building the HTML content
    strcpy(html, html_template_start);

    // Add each file as a list item with a link
    char *current_position = html + strlen(html); // Pointer to current position in the buffer

    for (size_t i = 0; i < files->length; i++) {
        int written = snprintf(current_position, total_length - (current_position - html), 
                               "<li><a href=\"%s\">%s</a></li>\n", 
                               files->file_names[i], files->file_names[i]);
        
        // Check if snprintf succeeded and did not exceed buffer size
        if (written < 0 || written >= (int)(total_length - (current_position - html))) {
            fprintf(stderr, "Error writing to HTML buffer\n");
            free(html);
            return;
        }

        current_position += written; // Move the pointer forward by the number of bytes written
    }

    // Append the closing tags
    strcat(current_position, html_template_end);

    // Send the constructed HTML back to the client
    send_html(socket, html);

    free(html); // Free allocated memory
}
