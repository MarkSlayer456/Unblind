#ifndef UNBLIND_H_

#define UNBLIND_H_

#include "double_linked_list.h"

#define LINES_PER_WINDOW 	          36
#define CHARS_PER_LINE_PER_WINDOW     90
#define PROTECTED_LINES 	          3
#define SCROLL_THRESHOLD	          (int)LINES_PER_WINDOW/6
#define SCROLLX_THRESHOLD	          (int)CHARS_PER_LINE_PER_WINDOW/10

#define PAGE_MOD		1200
#define PAGE_UP			PAGE_MOD + 53
#define PAGE_DOWN		PAGE_MOD + 54			

#define ARROW_KEY_MOD	1000
#define DOWN_ARROW		ARROW_KEY_MOD + 66
#define UP_ARROW		ARROW_KEY_MOD + 65
#define RIGHT_ARROW		ARROW_KEY_MOD + 67
#define LEFT_ARROW		ARROW_KEY_MOD + 68

#define CTRL_ARROW_KEY_MOD		1100
#define CTRL_DOWN_ARROW			CTRL_ARROW_KEY_MOD + 66
#define CTRL_UP_ARROW			CTRL_ARROW_KEY_MOD + 65
#define CTRL_RIGHT_ARROW		CTRL_ARROW_KEY_MOD + 67
#define CTRL_LEFT_ARROW			CTRL_ARROW_KEY_MOD + 68

#define BACKSPACE_KEY_0	127
#define BACKSPACE_KEY_1 8
#define BACKSPACE_KEY_2 7
#define ENTER_KEY		10
#define TAB_KEY 		9
#define CTRL_D			4
#define CTRL_X			24
#define CTRL_Z			26
#define CTRL_S			19
#define CTRL_Q			17
#define CTRL_P			16
#define CTRL_F			6

#define FIND_STR_MAX_LENGTH 300
#define MAX_MESSAGE_LENGTH 300

int MAX_LINES;
int MAX_CHARS_PER_LINE;
int INFO_SIZE;
int WINDOW_HEIGHT;

typedef enum {
	FIND = 1,
	EDIT = 2
} unblind_mode_t;

typedef struct user_settings {
	int tabsize;
	FILE *log;
} user_settings_t;

typedef struct unblind_info {
	int cx;
	int cy;
	int scroll_offset;
    int scrollX_offset;
	int wcx;
	int wcy;
	char **contents;
	char *message;
	d_linked_list_t *find;
	unblind_mode_t m;
	char *fstr;
	undo_redo_manager_t *ur_manager;
} unblind_info_t;

void read_contents_from_file(FILE *f, WINDOW *win, unblind_info_t *info);
void write_contents_to_file(char *file_name, unblind_info_t *info);
void manage_input(char *file_input, WINDOW *win, unblind_info_t *info);
void delete_line(WINDOW *win, unblind_info_t *info);
void draw(WINDOW *win, unblind_info_t *info);
void reset_unblind_info_contents(unblind_info_t *info);

void unblind_scroll_left(WINDOW *win, unblind_info_t *info);
void unblind_scroll_right(WINDOW *win, unblind_info_t *info);
void unblind_scroll_down(WINDOW *win, unblind_info_t *info);
void unblind_scroll_up(WINDOW *win, unblind_info_t *info);

void enlarge_lines_unblind_info(unblind_info_t *info);
void enlarge_characters_unblind_info(unblind_info_t *info);
void setup_unblind_info(unblind_info_t *info);
void unblind_info_free(unblind_info_t *info);

void shutdown();

void update_cursor_pos(WINDOW *win, unblind_info_t *info);

void print_to_log(const char *error);
int array_insert(char *a, int x, char c, int size);
void move_to_left(char *arr, int left);
void shift_up(WINDOW *win, unblind_info_t *info);
void shift_down(WINDOW *win, unblind_info_t *info);
void duplicate_line(WINDOW *win, unblind_info_t *info);
void unblind_scroll_check(WINDOW *win, unblind_info_t *info);

void unblind_scroll_hor_calc(WINDOW *win, unblind_info_t *info);

// these functions will
// be moved to another file at a later date
//d_linked_list_t *linked_list_d_create();
//void linked_list_d_add(d_linked_list_t *dll, void *value, int x, int y);
//dll_node_t *linked_list_d_get(d_linked_list_t *dll, int i);

#endif
