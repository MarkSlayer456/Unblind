#ifndef UNBLIND_H_

#define UNBLIND_H_

#define MAX_LINES 8196
#define MAX_CHARS_PER_LINE 256

enum directions{UP, DOWN, LEFT, RIGHT};

void read_contents_from_file(FILE *f, WINDOW *win);
void write_contents_to_file(char *file_name);
void manage_input(char *file_input, WINDOW *win);
int length(char *arr);
void update_cursor_pos(WINDOW *win);
void move_cursor(enum directions d, WINDOW *win);
void delete_line(WINDOW *win, int y);


#endif