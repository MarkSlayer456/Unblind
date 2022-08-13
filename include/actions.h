#ifndef ACTIONS_H_

#define ACTIONS_H_

#define TAB_SIZE 4

void jump_to_start(unblind_info_t *info);
void jump_to_end(unblind_info_t *info);

void jump_to_line(unblind_info_t *info, int line);
void jump_forward_word(unblind_info_t *info);
void jump_backward_word(unblind_info_t *info);

void move_cursor_up(unblind_info_t *info);
void move_cursor_down(unblind_info_t *info);
void move_cursor_left(unblind_info_t *info);
void move_cursor_right(unblind_info_t *info);
void move_cursor_to_message(unblind_info_t *info);

void find_str(unblind_info_t *info);
void next_find_str(unblind_info_t *info);
int hash(char *str);

void save_file(char *file_name, unblind_info_t *info);

void backspace_action(unblind_info_t *info, int add_to_ur_manager);
void enter_key_action(unblind_info_t *info, int add_to_ur_manager);
void type_char(char c, unblind_info_t *info, int add_to_ur_manager);
void tab_action(unblind_info_t *info, int add_to_ur_manager);
void delete_line(unblind_info_t *info, int add_to_ur_manager);
void duplicate_line(unblind_info_t *info, int add_to_ur_manager);
void move_line_down(unblind_info_t *info, int add_to_ur_manager);
void move_line_up(unblind_info_t *info, int add_to_ur_manager);


char current_character(unblind_info_t *info);
char next_character(unblind_info_t *info);
char prev_character(unblind_info_t *info);

char *current_line(unblind_info_t *info);
char *next_line(unblind_info_t *info);
char *prev_line(unblind_info_t *info);

void undo_type_char(unblind_info_t *info, int x, int y);
void undo_backspace(unblind_info_t *info, char *c, int x, int y);
void undo_last_backspace(unblind_info_t *info, char *c, int x, int y);
void undo_enter(unblind_info_t *info, char *c, int y);
void undo_tab(unblind_info_t *info, int x, int y);
void undo_delete_line(unblind_info_t *info, char *c, int x, int y);
void undo_duplicate_line(unblind_info_t *info, int x, int y);
void undo_move_line_down(unblind_info_t *info, int x, int y);
void undo_move_line_up(unblind_info_t *info, int x, int y);


#endif
