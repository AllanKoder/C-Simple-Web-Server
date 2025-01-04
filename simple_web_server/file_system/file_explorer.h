#ifndef FILE_EXPLORER
#define FILE_EXPLORER

#include <stdbool.h>
#include <stdlib.h>

enum FileType
{
    NONE,
    DIRECTORY,
    TXT,
    HTML,
    PNG,
    OTHER,
};

struct FileContent
{
    long size;
    char *bytes;
};

struct FilesList
{
    size_t length;
    char *file_names[];
};

struct FilesList *get_directory_files(const char *path);
void free_files_list(struct FilesList *files_list);
char *get_valid_path(const char *path, const char *path_to_serving_dir);
enum FileType get_file_type(const char *path);
char *get_string_content(const char *path);
struct FileContent get_bytes_content(const char *path);

#endif
