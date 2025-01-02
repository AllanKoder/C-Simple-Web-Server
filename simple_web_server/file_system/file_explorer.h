#ifndef FILE_EXPLORER
#define FILE_EXPLORER

#include <stdbool.h>

enum FileType {
    DIR,
    TXT,
    CGI,
    OTHER,
};

char *get_valid_file(char *path);
enum FileType get_file_type(char *path);
char *get_file_content(char *path);

#endif