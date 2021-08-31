#include "csv_parse.h"
/**
 * parses a given file
 * returns the filled out csv_data_t struct
 **/
csv_data_t parse(char *file_name)
{
	int f = open(file_name, O_RDONLY);
	
    if(f == -1) {
		printf("ERROR: UNKNOWN FILE: %s CAN NOT PARSE... EXITING PROGRAM\n", file_name);
		exit(1);
	}	
    int wholefile_size = K;
    char *wholefile = calloc(wholefile_size, sizeof(char));
	
	char *buf = calloc(K, sizeof(char));
    int r = read(f, buf, K);
	
    while(r > 0) {
        wholefile_size += K;
        wholefile = realloc(wholefile, sizeof(char) * wholefile_size);
        strncat(wholefile, buf, K);
		r = read(f, buf, K);
	}
	free(buf);
	
	
    int lines_size = 16;
    char **lines = malloc(sizeof(char *) * (lines_size));

	
    int i = 0;
	char *tok = strtok(wholefile, "\n");
	while(tok != NULL) {
        if(i >= lines_size) {
            lines_size *= 2;
            lines = realloc(lines, sizeof(char *) * lines_size);
		}
		
        lines[i] = malloc(strlen(tok)+1);
		strcpy(lines[i], tok);
		i++;
		tok = strtok(NULL, "\n");
	}
	
	int parse_size = i+1;
    char ***parse = malloc(sizeof(char **) * parse_size);

    int j = 0;
    int cur = 0;
	for(; j < i; j++) {
		int parse_size2 = 16;
        parse[j] = malloc(parse_size2 * sizeof(char *));
		tok = strtok(lines[j], ",");
		cur = 0;
		while(tok != NULL) {
            if(cur >= parse_size2) {
                parse_size2 *= 2;
                parse[j] = malloc(parse_size2 * sizeof(char *));
			}
			parse[j][cur] = malloc(strlen(tok)+1);
			strcpy(parse[j][cur], tok);
			cur++;
			tok = strtok(NULL, ",");
		}
	}

    csv_data_t csv;
    csv.data = parse;
    csv.cols = cur;
    csv.rows = j;
    close(f);
	free(wholefile);
	for(int j = 0; j < i; j++) {
		free(lines[j]);
	}
	free(lines);
	return csv;
}

/**
 * returns line number where item first occurs
 **/
int csv_get_line(csv_data_t csv, char *value)
{
    for(int j = 0; j < csv.cols; j++) {
        for(int i = 0; i < csv.rows; i++) {
            if(strcasecmp(csv.data[i][j], value) == 0) {
                return i;
            }
        }
    }
    return -1;
}

/**
 * returns the line of the first occurence of value
 **/
char **csv_get_line_from_value(csv_data_t csv, char *value) 
{
    int line = csv_get_line(csv, value);
    csv_get_line_data(csv, line);
    return NULL;
}

/**
 * returns all values from a specific line
 **/
char **csv_get_line_data(csv_data_t csv, int lineNum)
{
    return csv.data[lineNum];
}

/**
 * returns all the line numbers with the given value
 * the returned array is 0 terminated
 **/
int *csv_find_all(csv_data_t csv, char *value)
{
    int size = 10;
	int *found = malloc(sizeof(int) * size);
    memset(found, 0, size);
    int count = 0;
    for(int j = 0; j < csv.cols; j++) {
        for(int i = 0; i < csv.rows; i++) {
            if(strcasecmp(csv.data[i][j], value) == 0) {
                if(count == size) {
                    size *= 2;
                    found = realloc(found, sizeof(int) * size);
                    memset(found+(size/2), 0, size);
                }
                printf("found at: %d\n", i);
                found[count] = i;
                count++;
            }
        }
    }
    return found;
}
