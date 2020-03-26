#ifndef UNBLIND_H_

#define UNBLIND_H_

#define MAX_LINES 8196
#define MAX_CHARS_PER_LINE 1024
#define INFO_SIZE MAX_LINES * MAX_CHARS_PER_LINE * 2
#define LINES_PER_WINDOW 	36
#define PROTECTED_LINES 	3

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
#define CTRL_S			19
#define CTRL_Q			17

typedef struct user_settings {
	int tabsize;
	FILE *log;
} user_settings_t;

typedef struct unblind_info {
	int cx;
	int cy;
	char **contents;
	char *message;
	int scroll_offset;
	int wcx;
	int wcy;
} unblind_info_t;

void read_contents_from_file(FILE *f, WINDOW *win, unblind_info_t *info);
void write_contents_to_file(char *file_name, unblind_info_t *info);
void manage_input(char *file_input, WINDOW *win, unblind_info_t *info);
void delete_line(WINDOW *win, unblind_info_t *info);
void draw(WINDOW *win, unblind_info_t *info);

void unblind_scroll_down(WINDOW *win, unblind_info_t *info);
void unblind_scroll_up(WINDOW *win, unblind_info_t *info);

void setup_unblind_info(unblind_info_t *info);
void unblind_info_free(unblind_info_t *info);

void shutdown();

void update_cursor_pos(WINDOW *win, unblind_info_t *info);

void print_to_log(const char *error);
int array_insert(char *a, int x, char c);
void move_to_left(char *arr, int left);
void shift_up(WINDOW *win, unblind_info_t *info);
void shift_down(WINDOW *win, unblind_info_t *info);
void duplicate_line(WINDOW *win, unblind_info_t *info);
void unblind_scroll_check(WINDOW *win, unblind_info_t *info);


#endif
