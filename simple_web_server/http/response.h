#ifndef RESPONSE
#define RESPONSE

#include "config.h"
#include <stdlib.h>

void send_404_page(int socket);

/**
 * Sending documents to be rendered as html
 */
void send_html(int socket, const char *html);

/**
 * Sending documents to be rendered as text
 */
void send_text(int socket, const char *text);


#endif