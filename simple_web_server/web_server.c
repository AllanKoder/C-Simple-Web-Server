#include "config.h"
#include "web_server.h"
#include "request.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

// Networks
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>

#define RESPONSE_MESSAGE "HTTP/1.1 200 OK\r\n\
Access-Control-Allow-Origin: *\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/html; charset=utf-8\r\n\
\r\n\
Hello World"


void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    (void)s;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int create_server_socket()
{
    int server_socket;
    
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socketfd");
        exit(EXIT_FAILURE);
    }
    
    return server_socket;
}

void configure_socket(int server_socket)
{
    struct sockaddr_in hints;
    struct sigaction sa;

    // Reuse socket
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Handle zombies
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    hints.sin_family = AF_INET;
    hints.sin_addr.s_addr = INADDR_ANY;
    hints.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&hints, sizeof(hints)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, BACKLOG) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

void accept_connections(int server_socket)
{
    struct sockaddr_in client_address;
    socklen_t client_socket_size;

    while (1)
    {
        int client_socket;
        client_socket_size = sizeof(client_address);
        
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_socket_size)) == -1)
        {
            perror("accept");
            continue; // Continue to the next iteration on error
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            close(server_socket); // Close the listening socket in the child process
            handle_client(client_socket);
            exit(EXIT_SUCCESS);
        }
        else if (pid > 0)
        {
            // Parent process
            close(client_socket); // Close the connected socket in the parent process
            printf("Parent process: Child %d finished\n", pid);
        }
        else
        {
            perror("fork");
            close(client_socket); // Close the socket in case of fork failure
        }
    }
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE] = {0};
    struct HttpRequest request;
    
    int valread = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    if (valread < 0)
    {
        perror("recv");
        close(client_socket);
        return; // Exit the function on error
    }
    
    buffer[valread] = '\0'; // Null-terminate the buffer
    printf("Received Request:\n%s\n", buffer);

    // Parse the content
    parse_http_request(buffer, &request);

    // Hande the response

    ssize_t bytes_sent = send(client_socket, RESPONSE_MESSAGE, sizeof(RESPONSE_MESSAGE) - 1, 0);
    
    if (bytes_sent == -1)
    {
        perror("send");
    }
    else if ((size_t)bytes_sent < sizeof(RESPONSE_MESSAGE) - 1)
    {
        fprintf(stderr, "Partial send\n");
    }

    close(client_socket);
}
