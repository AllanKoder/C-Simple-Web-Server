#include "file_explorer.h"
#include "config.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

char *get_valid_path(const char *path, const char *path_to_serving_dir)
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

    // If the original path ends with '/', append it to the resolved path
    if (path[strlen(path) - 1] == '/')
    {
        size_t abs_length = strlen(absolute_path);
        // Allocate new memory for the modified path with trailing slash
        char *valid_path = malloc(abs_length + 2); // +1 for '/' +1 for '\0'
        if (valid_path == NULL)
        {
            perror("malloc");
            free(absolute_path);
            free(serving_directory);
            return NULL;
        }
        strcpy(valid_path, absolute_path);
        valid_path[abs_length] = '/';      // Add trailing slash
        valid_path[abs_length + 1] = '\0'; // Null-terminate

        free(absolute_path);     // Free original absolute path
        free(serving_directory); // Free serving directory
        return valid_path;       // Return new valid path with trailing slash
    }

    // Free current_directory as we no longer need it
    free(serving_directory);
    return absolute_path; // Caller is responsible for freeing this
}

enum FileType get_file_type(const char *path)
{
    // Check if the path is NULL or empty
    if (path == NULL || strlen(path) == 0)
    {
        return NONE;
    }

    // Check if the path ends with a '/'
    if (path[strlen(path) - 1] == '/')
    {
        return DIRECTORY; // Directly return DIRECTORY if it ends with '/'
    }

    // Find the last occurrence of '.' to determine the file extension
    char *file_type = strrchr(path, '.');

    // If there is no '.' in the file name, return OTHER
    if (file_type == NULL)
    {
        return OTHER;
    }

    // Move past the '.' to get the extension
    file_type++;

    // Compare extensions to determine file type
    if (strcmp(file_type, "png") == 0)
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

    return OTHER; // Return OTHER for unrecognized extensions
}

struct FilesList *get_directory_files(const char *path) {
    DIR *dir;
    struct dirent *entry;
    size_t count = 0;

    // Open the directory
    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return NULL; // Return NULL on error
    }

    // First pass: Count the number of files and directories
    while ((entry = readdir(dir)) != NULL) {
        // Skip special entries "." and ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            count++;
        }
    }

    // Allocate memory for FilesList (including space for file_names)
    struct FilesList *files_list = malloc(sizeof(struct FilesList) + count * sizeof(char *));
    if (files_list == NULL) {
        perror("malloc");
        closedir(dir);
        return NULL; 
    }

    // Reset directory stream to read file names
    rewinddir(dir);

    // Second pass: Store file and directory names
    size_t index = 0;
    while ((entry = readdir(dir)) != NULL) {
        // Skip special entries "." and ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Check if it's a directory
            if (entry->d_type == DT_DIR) {
                // Allocate memory for directory name plus trailing slash
                size_t dir_name_length = strlen(entry->d_name) + 2; // +1 for '/' +1 for '\0'
                files_list->file_names[index] = malloc(dir_name_length);
                if (files_list->file_names[index] == NULL) {
                    perror("malloc");
                    closedir(dir);
                    for (size_t i = 0; i < index; i++) { 
                        free(files_list->file_names[i]);
                    }
                    free(files_list); 
                    return NULL;      
                }
                snprintf(files_list->file_names[index], dir_name_length, "%s/", entry->d_name); // Append '/'
            } else { // It's a regular file
                files_list->file_names[index] = strdup(entry->d_name); // Duplicate file name
            }

            if (files_list->file_names[index] == NULL) {
                perror("strdup");
                closedir(dir);
                for (size_t i = 0; i < index; i++) { 
                    free(files_list->file_names[i]);
                }
                free(files_list); 
                return NULL;      
            }
            printf("File/Directory name: %s\n", files_list->file_names[index]);
            index++;
        }
    }

    closedir(dir);
    files_list->length = count;

    printf("Returned with %zu entries.\n", files_list->length);
    return files_list; 
}


void free_files_list(struct FilesList *files_list)
{
    if (files_list != NULL)
    {
        for (size_t i = 0; i < files_list->length; i++)
        {
            free(files_list->file_names[i]); // Free each file name
        }
        free(files_list); // Free the FilesList structure itself
    }
}

char *get_string_content(const char *path)
{
    FILE *fd = fopen(path, "r");
    char *response = NULL; // Initialize response to NULL

    if (fd)
    {
        fseek(fd, 0, SEEK_END);
        long length = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        response = malloc(length + 1);
        if (response)
        {
            size_t read_bytes = fread(response, 1, length, fd);
            response[read_bytes] = '\0'; // Null-terminate only if read was successful
        }

        fclose(fd);
    }
    else
    {
        perror("Error opening file");
    }

    if (response)
    {
        printf("File content: %s\n", response);
    }
    else
    {
        printf("Failed to read file content.\n");
    }

    return response; // Return allocated memory or NULL
}

struct FileContent get_bytes_content(const char *path)
{
    FILE *fd = fopen(path, "rb");
    char *response = NULL;
    struct FileContent content;

    if (fd)
    {
        fseek(fd, 0, SEEK_END);
        long length = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        response = malloc(length + 1); // Allocate memory for binary data
        if (response)
        {
            fread(response, 1, length, fd);
            content.bytes = response; // Assign bytes only if read was successful
            content.size = length;
        }
        else
        {
            content.bytes = NULL;
            content.size = 0;
        }

        fclose(fd);
    }
    else
    {
        perror("Error opening file");
        content.bytes = NULL;
        content.size = 0;
    }

    printf("File content size: %ld\n", content.size); // Print size instead of bytes
    return content;                                   // Return struct with size and bytes
}
