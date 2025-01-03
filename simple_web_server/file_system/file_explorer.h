#ifndef FILE_EXPLORER
#define FILE_EXPLORER

#include <stdbool.h>

enum FileType
{
    DIR,
    TXT,
    HTML,
    PNG,
    CGI,
    OTHER,
};

struct FileContent
{
    long size;
    char *bytes;
};

char *get_valid_path(char *path, const char *path_to_serving_dir);
enum FileType get_file_type(char *path);
char *get_string_content(char *path);
struct FileContent get_bytes_content(char *path);

#endif
