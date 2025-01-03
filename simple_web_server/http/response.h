#ifndef RESPONSE
#define RESPONSE

#include "config.h"
#include <stdlib.h>

/**
 * Sending the 404 page not found response
 */
void send_404_page(int socket);

/**
 * Sending documents to be rendered as html
 */
void send_html(int socket, const char *html);

/**
 * Sending documents to be rendered as text
 */
void send_text(int socket, const char *text);

/**
 * Send the response for displaying a png
 */
void send_png(int socket, const char *filename, struct FileContent content);

/**
 * Send the response for downloading the file
 */
void send_download(int socket, const char *filename, struct FileContent content);

#endif