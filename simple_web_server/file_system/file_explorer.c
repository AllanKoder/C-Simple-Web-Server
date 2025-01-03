#include "file_explorer.h"
#include "config.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *get_valid_path(char *path, const char *path_to_serving_dir)
{
    // Get the server directory
    char *serving_directory = realpath(path_to_serving_dir, NULL);
    if (serving_directory == NULL)
    {
        perror("Failed to get serving directory");
        return NULL;
    }

    // Resolve the absolute path of the requested file
    char *absolute_path = realpath(path, NULL);
    if (absolute_path == NULL)
    {
        perror("Failed to resolve absolute path");
        free(serving_directory);
        return NULL;
    }

    size_t length = strlen(serving_directory);

    // Check if the resolved absolute path starts with the current directory
    if (strncmp(serving_directory, absolute_path, length) != 0)
    {
        printf("Not a valid path, path traversal was attempted\n");
        free(absolute_path); // Free allocated memory for absolute_path
        free(serving_directory);
        return NULL;
    }

    // Free current_directory as we no longer need it
    free(serving_directory);
    return absolute_path; // Caller is responsible for freeing this
}

enum FileType get_file_type(char *path)
{
    char *file = strrchr(path, '/');
    if (file == NULL)
    {
        return DIR;
    }
    char *file_type = strrchr(path, '.')+1;
    if (file_type == NULL)
    {
        return DIR;
    }
    else if (strcmp(file_type, "png") == 0)
    {
        return PNG;
    }
    else if (strcmp(file_type, "html") == 0)
    {
        return HTML;
    }
    else if (strcmp(file_type, "txt") == 0)
    {
        return TXT;
    }
    else if (strcmp(file_type, "cgi") == 0)
    {
        return CGI;
    }
    return OTHER;
}

char *get_string_content(char *path) {
    FILE *fd = fopen(path, "r");
    char *response = NULL; // Initialize response to NULL

    if (fd) {
        fseek(fd, 0, SEEK_END);
        long length = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        response = malloc(length + 1);
        if (response) {
            size_t read_bytes = fread(response, 1, length, fd);
            response[read_bytes] = '\0'; // Null-terminate only if read was successful
        }

        fclose(fd);
    } else {
        perror("Error opening file");
    }

    if (response) {
        printf("File content: %s\n", response);
    } else {
        printf("Failed to read file content.\n");
    }

    return response; // Return allocated memory or NULL
}

struct FileContent get_bytes_content(char *path) {
    FILE *fd = fopen(path, "rb");
    char *response = NULL;
    struct FileContent content;

    if (fd) {
        fseek(fd, 0, SEEK_END);
        long length = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        response = malloc(length + 1); // Allocate memory for binary data
        if (response) {
            size_t read_bytes = fread(response, 1, length, fd);
            content.bytes = response; // Assign bytes only if read was successful
            content.size = length;
        } else {
            content.bytes = NULL;
            content.size = 0;
        }

        fclose(fd);
    } else {
        perror("Error opening file");
        content.bytes = NULL;
        content.size = 0;
    }

    printf("File content size: %ld\n", content.size); // Print size instead of bytes
    return content; // Return struct with size and bytes
}
