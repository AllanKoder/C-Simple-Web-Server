#include "web_server.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int server_socket = create_server_socket();
    configure_socket(server_socket);

    if (argc < 2)
    {
        printf("No directory specified, assuming current directory is where files will be stored.\n");
        accept_connections(server_socket, ".");
    }
    else
    {
        printf("Web server dir: %s\n", argv[1]);
        accept_connections(server_socket, argv[1]);
    }

    return EXIT_SUCCESS;
}