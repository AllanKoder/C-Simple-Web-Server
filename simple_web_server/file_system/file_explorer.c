#include "file_explorer.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *get_valid_file(char *path) {
    // Get the current directory
    char *current_directory = realpath("./", NULL);
    if (current_directory == NULL) {
        perror("Failed to get current directory");
        return NULL;
    }

    // Resolve the absolute path of the requested file
    char *absolute_path = realpath(path, NULL);
    if (absolute_path == NULL) {
        perror("Failed to resolve absolute path");
        free(current_directory); 
        return NULL;
    }

    size_t length = strlen(current_directory);
    
    // Check if the resolved absolute path starts with the current directory
    if (strncmp(current_directory, absolute_path, length) != 0) {
        printf("Not a valid path, path traversal was attempted\n");
        free(absolute_path); // Free allocated memory for absolute_path
        free(current_directory); 
        return NULL;
    }

    // Free current_directory as we no longer need it
    free(current_directory); 
    return absolute_path; // Caller is responsible for freeing this
}

enum FileType get_file_type(char *path)
{
    char* file = strrchr(path, '/');
    if (file == NULL)
    {
        return DIR;
    }
    char* file_type = strrchr(path, '.');
    if (file_type == NULL)
    {
        return DIR;
    }
    else if (strcmp(file_type,"html"))
    {
        return HTML;
    }
    else if (strcmp(file_type,"txt"))
    {
        return TXT;
    }
    else if (strcmp(file_type,"cgi"))
    {
        return CGI;
    }
    return OTHER;
}