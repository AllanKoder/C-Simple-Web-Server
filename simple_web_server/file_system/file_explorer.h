#ifndef FILE_EXPLORER
#define FILE_EXPLORER

#include <stdbool.h>

enum FileType {
    DIR,
    TXT,
    HTML,
    PNG,
    CGI,
    OTHER,
};

char *get_valid_path(char *path, const char *serving_dir);
enum FileType get_file_type(char *path);
char *get_file_content(char *path, const char *options);

#endif