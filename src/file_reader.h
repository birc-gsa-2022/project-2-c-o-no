#ifndef FILE_READER_H
#define FILE_READER_H
#include <malloc.h>

char *read_file(const char *file_name) {
    FILE *fp;
    fp = fopen(file_name, "rb+");

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, fp);
    fclose(fp);

    string[fsize] = '\0'; // terminate with zero
    return string;
}

#endif //FILE_READER_H
