#ifndef ACTIONS_H_

#define ACTIONS_H_

void move_cursor_up(WINDOW *win, unblind_info_t *info);
void move_cursor_down(WINDOW *win, unblind_info_t *info);
void move_cursor_left(WINDOW *win, unblind_info_t *info);
void move_cursor_right(WINDOW *win, unblind_info_t *info);

void backspace_action(WINDOW *win, unblind_info_t *info);
void enter_key_action(WINDOW *win, unblind_info_t *info);
void save_file(char *file_name, unblind_info_t *info);
void type_char(char c, unblind_info_t *info);
void tab_action(WINDOW *win, unblind_info_t *info);

#endif