#ifndef ACTIONS_H_

#define ACTIONS_H_

#define TAB_SIZE 4

void jump_to_start(WINDOW *win, unblind_info_t *info);
void jump_to_end(WINDOW *win, unblind_info_t *info);

void jump_to_line(WINDOW *win, unblind_info_t *info, int line);

void move_cursor_up(WINDOW *win, unblind_info_t *info);
void move_cursor_down(WINDOW *win, unblind_info_t *info);
void move_cursor_left(WINDOW *win, unblind_info_t *info);
void move_cursor_right(WINDOW *win, unblind_info_t *info);
void move_cursor_to_message(WINDOW *win, unblind_info_t *info);

void find_str(WINDOW *win, unblind_info_t *info);
void next_find_str(WINDOW *win, unblind_info_t *info);
int hash(char *str);

void save_file(char *file_name, unblind_info_t *info);

void backspace_action(WINDOW *win, unblind_info_t *info, int add_to_ur_manager);
void enter_key_action(WINDOW *win, unblind_info_t *info, int add_to_ur_manager);
void type_char(WINDOW *win, char c, unblind_info_t *info, int add_to_ur_manager);
void tab_action(WINDOW *win, unblind_info_t *info, int add_to_ur_manager);
void delete_line(WINDOW *win, unblind_info_t *info, int add_to_ur_manager);
void duplicate_line(WINDOW *win, unblind_info_t *info, int add_to_ur_manager);
void move_line_down(WINDOW *win, unblind_info_t *info, int add_to_ur_manager);
void move_line_up(WINDOW *win, unblind_info_t *info, int add_to_ur_manager);


char current_character(unblind_info_t *info);
char next_character(unblind_info_t *info);
char prev_character(unblind_info_t *info);

char *current_line(unblind_info_t *info);
char *next_line(unblind_info_t *info);
char *prev_line(unblind_info_t *info);

void undo_type_char(WINDOW *win, unblind_info_t *info, int x, int y);
void undo_backspace(WINDOW *win, unblind_info_t *info, char *c, int x, int y);
void undo_last_backspace(WINDOW *win, unblind_info_t *info, char *c, int x, int y);
void undo_enter(WINDOW *win, unblind_info_t *info, char *c, int y);
void undo_tab(WINDOW *win, unblind_info_t *info, int x, int y);
void undo_delete_line(WINDOW *win, unblind_info_t *info, char *c, int x, int y);
void undo_duplicate_line(WINDOW *win, unblind_info_t *info, int x, int y);
void undo_move_line_down(WINDOW *win, unblind_info_t *info, int x, int y);
void undo_move_line_up(WINDOW *win, unblind_info_t *info, int x, int y);


#endif
