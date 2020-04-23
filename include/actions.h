#ifndef ACTIONS_H_

#define ACTIONS_H_

void move_cursor_up(WINDOW *win, unblind_info_t *info);
void move_cursor_down(WINDOW *win, unblind_info_t *info);
void move_cursor_left(WINDOW *win, unblind_info_t *info);
void move_cursor_right(WINDOW *win, unblind_info_t *info);

void find_str(WINDOW *win, unblind_info_t *info);
void next_find_str(WINDOW *win, unblind_info_t *info);

void backspace_action(WINDOW *win, unblind_info_t *info);
void enter_key_action(WINDOW *win, unblind_info_t *info);
void save_file(char *file_name, unblind_info_t *info);
void type_char(char c, unblind_info_t *info);
void tab_action(WINDOW *win, unblind_info_t *info);

char current_character(unblind_info_t *info);
char next_character(unblind_info_t *info);
char prev_character(unblind_info_t *info);

char *current_line(unblind_info_t *info);
char *next_line(unblind_info_t *info);
char *prev_line(unblind_info_t *info);

#endif
