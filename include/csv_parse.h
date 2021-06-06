#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#ifndef CSV_PARSE_H_
#define CSV_PARSE_H_

#define K 1024

typedef struct csv_data {
    char ***data;
    int rows;
    int cols;
} csv_data_t;

csv_data_t parse(char *file_name);

char *get_col_names(char ***data);

int csv_get_line(csv_data_t csv, char *value);
char **csv_get_line_data(csv_data_t csv, int lineNum);

char **csv_get_line_from_value(csv_data_t csv, char *value);

int *csv_find_all(csv_data_t csv, char *value);

#endif
