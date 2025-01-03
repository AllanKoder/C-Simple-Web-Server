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
    char *file_type = strrchr(path, '.');
    if (file_type == NULL)
    {
        return DIR;
    }
    else if (strcmp(file_type, "html"))
    {
        return HTML;
    }
    else if (strcmp(file_type, "txt"))
    {
        return TXT;
    }
    else if (strcmp(file_type, "cgi"))
    {
        return CGI;
    }
    return OTHER;
}

char *get_file_content(char *path)
{
    FILE *fd = fopen(path, "r");
    char *response;
    if (fd)
    {
        fseek(fd, 0, SEEK_END);
        long length = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        response = malloc(length + 1);

        if (fd == NULL)
        {
            return NULL;
        }
        if (response)
        {
            fread(response, 1, length, fd);
        }
        response[length] = 0;
        fclose(fd);
    }

    printf("File content: %s\n", response);
    return response;
}