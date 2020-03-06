#ifndef UNBLIND_H_

#define UNBLIND_H_

#define MAX_LINES 8196
#define MAX_CHARS_PER_LINE 255

enum directions{UP, DOWN, LEFT, RIGHT};

void read_contents_from_file(FILE *f, WINDOW *win);
void write_contents_to_file(char *file_name);
void manage_input(char *file_input, WINDOW *win);
int length(char *arr);
void update_cursor_pos(WINDOW *win);
void move_cursor(enum directions d, WINDOW *win);
void delete_line(WINDOW *win, int y);
void draw(WINDOW *win);
int str_insert(char *arr, int insert, char c);
void array_insert(char *a, int x, char c);
void move_to_left(char *arr, int left);
char *array_merge(char *arr1, char *arr2);
void shutdown();


#endif