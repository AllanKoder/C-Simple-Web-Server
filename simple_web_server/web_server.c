#include "config.h"
#include "web_server.h"
#include "file_explorer.h"
#include "request.h"
#include "response.h"

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

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    (void)s;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
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

void accept_connections(int server_socket, const char *directory)
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
            handle_client(client_socket, directory);
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

void handle_client(int client_socket, const char *directory) {
    char buffer[BUFFER_SIZE] = {0};
    struct HttpRequest request;

    // Receive data from the client
    int valread = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (valread < 0) {
        perror("recv");
        close(client_socket);
        return; // Exit on error
    }

    buffer[valread] = '\0'; // Null-terminate the buffer
    printf("Received Request:\n%s\n", buffer);

    // Parse the HTTP request
    parse_http_request(buffer, &request);

    // Get the requested file
    char *requested_file = get_requested_file(&request, directory);
    
    if (requested_file == NULL) {
        fprintf(stderr, "Failed to get requested file\n");
        close(client_socket);
        return;
    }

    printf("Requested file: %s\n", requested_file);
        
    char *full_filepath = get_valid_path(requested_file, directory);
    
    if (full_filepath == NULL) {
        fprintf(stderr, "Invalid file path\n");
        send_404_page(client_socket); // Send file not found

        free(requested_file); // Free the requested file
        close(client_socket);
        return;
    }

    char *filename = strrchr(full_filepath, '/')+1;
    printf("File: %s\n", full_filepath);

    // Determine File type and perform logic for each type
    enum FileType file_type = get_file_type(full_filepath);
    printf("File type: %d\n", file_type);

    char *file_content;
    struct FileContent file_bytes;
    switch(file_type) {
        case DIR:
            file_content = get_string_content(full_filepath);
            send_404_page(client_socket);
            free(file_content); // Free file content
            break;
        case TXT:
            file_content = get_string_content(full_filepath);
            if (file_content == NULL)
            {
                send_text(client_socket, "Error opening file");
            }
            else
            {
                send_text(client_socket, file_content);
            }
            free(file_content); // Free file content
            break;
        case HTML:
            file_content = get_string_content(full_filepath);
            if (file_content == NULL)
            {
                send_html(client_socket, "Error opening file");
            }
            else
            {
                send_html(client_socket, file_content);
            }
            free(file_content); // Free file content
            break;
        case CGI:
            // Handle CGI logic here
            break;
        case PNG:
            file_bytes = get_bytes_content(full_filepath);
            if (file_bytes.bytes) {
                send_png(client_socket, filename, file_bytes);
                free(file_bytes.bytes); // Free allocated memory after sending
            } else {
                printf("Failed to read PNG file.\n");
                send_404_page(client_socket); // Handle error appropriately
            }
            break;
        case OTHER:
            file_bytes = get_bytes_content(full_filepath);
            // Handle other file types here
            send_download(client_socket, filename, file_bytes);
            free(file_bytes.bytes);
            break;
        default:
            send_404_page(client_socket);
            break;
    }

    free(requested_file); // Free requested_file after use
    free(full_filepath); // Free full_filepath after use
    close(client_socket); // Close client socket connection
}
