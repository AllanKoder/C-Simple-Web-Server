#include "web_server.h"
#include <stdlib.h>

int main()
{
    int server_socket = create_server_socket();
    configure_socket(server_socket);
    accept_connections(server_socket);
    return EXIT_SUCCESS;
}