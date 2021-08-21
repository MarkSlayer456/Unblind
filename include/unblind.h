#ifndef UNBLIND_H_

#define UNBLIND_H_

#include "double_linked_list.h"

#define THREADS                       2

#define LINES_PER_WINDOW 	          36
#define CHARS_PER_LINE_PER_WINDOW     90
#define PROTECTED_LINES 	          3
#define MAX_WINDOWS                   4

#define PAGE_MOD		              1200
#define PAGE_UP			              PAGE_MOD + 53
#define PAGE_DOWN		              PAGE_MOD + 54			

#define ARROW_KEY_MOD	              1000
#define DOWN_ARROW		              ARROW_KEY_MOD + 66
#define UP_ARROW		              ARROW_KEY_MOD + 65
#define RIGHT_ARROW		              ARROW_KEY_MOD + 67
#define LEFT_ARROW		              ARROW_KEY_MOD + 68

#define CTRL_ARROW_KEY_MOD		      1100
#define CTRL_DOWN_ARROW			      CTRL_ARROW_KEY_MOD + 66
#define CTRL_UP_ARROW			      CTRL_ARROW_KEY_MOD + 65
#define CTRL_RIGHT_ARROW		      CTRL_ARROW_KEY_MOD + 67
#define CTRL_LEFT_ARROW			      CTRL_ARROW_KEY_MOD + 68

#define BACKSPACE_KEY_0	              127
#define BACKSPACE_KEY_1               8
#define BACKSPACE_KEY_2               7
#define ENTER_KEY		              10
#define TAB_KEY 		              9
#define CTRL_D			              4
#define CTRL_X			              24
#define CTRL_Z			              26
#define CTRL_S			              19
#define CTRL_Q			              17
#define CTRL_P			              16
#define CTRL_F		                  6
#define CTRL_B                        2
#define CTRL_W                        23
#define CTRL_E                        5
#define ESC_KEY						  27

#define FIND_STR_MAX_LENGTH           300
#define MAX_MESSAGE_LENGTH            300
#define MAX_JUMP_STR_LENGTH           300

extern int MAX_LINES;
extern int MAX_CHARS_PER_LINE;
extern int INFO_SIZE;
extern int WINDOW_HEIGHT;

int MAX_LINES;
int MAX_CHARS_PER_LINE;
int INFO_SIZE;
int WINDOW_HEIGHT;

typedef enum {
	RED = 1,
	BLUE = 2,
	GREEN = 3,
	MAGENTA = 4
} color_t;

typedef enum {
	UNKNOWN = 0,
	C = 1,
	JS = 2,
	PYTHON = 3,
	JAVA = 4
} language_t;

typedef enum {
    FIND = 1,
    EDIT = 2,
    INSERT = 3,
    JUMP = 4,
    CMD = 5,
	QUIT_SAVE = 6
} unblind_mode_t;

typedef struct parse_data {
	char **words;
	int size;
	color_t *colors;
	int colorCount;
	int wordCount;
} parse_data_t;

// this struct is getting pretty big
typedef struct unblind_info {
    int winlines;
    int wincols;
    WINDOW *win;
    char *file_name;
	int needs_saved;
	int prompt_save;
    
	parse_data_t *p_data;
	
    int x_threshold;
    int y_threshold;
    
    int cx;
    int cy;
    int scroll_offset;
    int scrollX_offset;
    int wcx;
    int wcy;
    
    char **contents;
    int *size;
    char *message;
    
    char *cmd;
    
    d_linked_list_t *find;
    unblind_mode_t m;
    char *fstr; // find string
    char *jstr; // jump string
    
    undo_redo_manager_t *ur_manager;
} unblind_info_t;

typedef struct user_settings {
    int tabsize;
    FILE *log;
} user_settings_t;

typedef struct thread_info {
    int windows;
    int activeWin;
    int drawAll;
    unblind_info_t **infos;
    pthread_mutex_t *lock;
} th_info_t;

// file.c
void read_contents_from_file(FILE *f, unblind_info_t *info);
void write_contents_to_file(char *file_name, unblind_info_t *info);

// unblind.c
void unblind_move_to_message(unblind_info_t *info);
void update_cursor_pos(unblind_info_t *info);
void manage_input(char *file_input, unblind_info_t *info, char c, th_info_t *th);
void draw(unblind_info_t *info);

// utils.c
language_t get_file_type(unblind_info_t *info);
color_t get_color(char *color);
int array_insert(char *a, int x, char c, int size);
void move_to_left(char *arr, int left, int size);
void shift_up(unblind_info_t *info);
void shift_down(unblind_info_t *info);

void unblind_scroll_hor_calc(unblind_info_t *info);
void unblind_scroll_vert_calc(unblind_info_t *info);

void change_active_window(th_info_t *th);
void set_active_window(th_info_t *th, int value);
void draw_all_screens(th_info_t *th);

//main.c
void inputThread();
void drawThread();
int create_win(th_info_t *th);
int close_active_win(th_info_t *th);

void update_window_size(th_info_t *th);

#endif
