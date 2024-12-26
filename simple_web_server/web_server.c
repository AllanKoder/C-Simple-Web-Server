#include "config.h"

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

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

int main(void)
{
    int server_socket;
    struct sockaddr_in hints, client_address;
    char buffer[BUFFER_SIZE] = {0};
    struct sigaction sa;
    socklen_t client_socket_size;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socketfd");
        exit(EXIT_FAILURE);
    }

    // Reuse socket
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) == -1)
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
    hints.sin_port = htons(8888);

    if (bind(server_socket, (struct sockaddr *)&hints, sizeof hints) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, BACKLOG) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        int client_socket;
        client_socket_size = sizeof(client_address);
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_socket_size)) == -1)
        {
            perror("accept");
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            printf("Child process %d created\n", getpid());
            close(server_socket);

            int valread = recv(client_socket, buffer, BUFFER_SIZE, 0);

            printf("Recieved Request:\n%s\n", buffer);

            ssize_t bytes_sent = send(client_socket, RESPONSE_MESSAGE, sizeof RESPONSE_MESSAGE, 0);
            if (bytes_sent == -1)
            {
                perror("send");
            }
            else if ((size_t)bytes_sent < sizeof RESPONSE_MESSAGE)
            {
                fprintf(stderr, "Partial send\n");
            }

            close(client_socket);
            printf("Child process %d exiting\n", getpid());
            exit(EXIT_SUCCESS);
        }
        else if (pid > 0)
        {
            // Parent process
            close(client_socket);
            printf("Parent process: Child %d finished\n", pid);
        }
        else
        {
            perror("fork");
        }
    }
}
