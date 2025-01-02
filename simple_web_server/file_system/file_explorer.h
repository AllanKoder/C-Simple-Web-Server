#ifndef FILE_EXPLORER
#define FILE_EXPLORER

#include <stdbool.h>

enum FileType {
    DIR,
    TXT,
    HTML,
    CGI,
    OTHER,
};

char *get_valid_file(char *path);
enum FileType get_file_type(char *path);

#endif