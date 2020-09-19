#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "double_linked_list.h"
#include "unblind.h"
#include "actions.h"

void unblind_scroll_check(WINDOW *win, unblind_info_t *info) {
	if(info->wcy <= SCROLL_THRESHOLD && info->scroll_offset > 0
		&& !(info->cy+1 <= -1)) {
		unblind_scroll_up(win, info);
	} else if(LINES_PER_WINDOW - SCROLL_THRESHOLD <= info->wcy && info->contents[info->wcy-1][0] != '\0'
				&& !(info->cy-1 >= MAX_LINES)) {
		unblind_scroll_down(win, info);
	}
}

void unblind_scroll_down(WINDOW *win, unblind_info_t *info) {
	info->wcy--;
	info->scroll_offset++;
	update_cursor_pos(win, info);
	draw(win, info);
}

void unblind_scroll_up(WINDOW *win, unblind_info_t *info) {
	info->wcy++;
	info->scroll_offset--;
	update_cursor_pos(win, info);
	draw(win, info);
}

void draw(WINDOW *win, unblind_info_t *info) {
	if(!info->contents) return;
	werase(win); //this was causing errors with the windows ubuntu system
	int y;
	for(int i = info->scroll_offset; i < MAX_LINES; i++) {
		y = i - info->scroll_offset;
		int x = 0;
		for(int j = 0; j <= strlen(info->contents[i]); j++) {
			if(info->contents[i][j] != '\0') {
				// bottom two lines are used for messages and other things
				if(y <= LINES-PROTECTED_LINES) {
				    wmove(win, y, x);
				    if(info->contents[i][j] == 9) {
						/*for(int i = 0; i < tab_size; i++) {
						TODO add later
						}*/
						waddch(win, ' ');
				    } else {
						waddch(win, info->contents[i][j]);
					}
				}
			}
			x++;
		}
	}
	print_to_log("done drawing...");
	wmove(win, LINES-2, 0);
	mvprintw(LINES-2, 0, info->message);
	wrefresh(win);
	update_cursor_pos(win, info);
}

void read_contents_from_file(FILE *f, WINDOW *win, unblind_info_t *info) {
    char c;
    int i = 0;
    int j = 0;
	print_to_log("reading contents from file");
    while((c = getc(f)) != EOF) {
        if(c == '\n') {
            // add char
            info->contents[j][i] = c;
			info->contents[j][i+1] = '\0';
            j++;
            i = 0;
        } else if(c == '\0') {
			if(i == 0) {
				info->contents[j][i] = '\n';
				info->contents[j][i+1] = '\0';
				j++;
				i = 0;
			} else {
				info->contents[j][i] = '\0';
				j++;
				i = 0;
			}
		} else if((c >= 32 && c <= 126) || c == 9) { // 9 is tab
            // add char
            if(c == 9) {
				for(int k = 0; k < 4; k++) {
					info->contents[j][i] = 9;
					i++;
				}
            } else {
            	info->contents[j][i] = c;
           		i++;
            }
        } else {
			info->contents[j][i] = c;
			info->contents[j][i+1] = '\0';
            j++;
            i = 0;
		}
    }
    fclose(f);
	print_to_log("moving to draw");
    draw(win, info);
}

void write_contents_to_file(char *file_name, unblind_info_t *info) {
    FILE *fedit = fopen(file_name, "w+");
    for(int i = 0; i < MAX_LINES; i++) {
		char *str = info->contents[i];
		for(int j = 0; j < strlen(str); j++) {
			if(strlen(str) == 0) {
				fputc('\n', fedit);
				continue;
			}
			char c = info->contents[i][j];
			if(c == 9) {
				j += 3;
				fputc(c, fedit);
			} else {
				fputc(c, fedit);
			}
		}
    }
    fclose(fedit);
}

void update_cursor_pos(WINDOW *win, unblind_info_t *info) {
	print_to_log("updating cursor pos...");
    mvprintw(LINES-1, COLS - 13, "pos: %d, %d ", info->cx, info->cy);
    mvprintw(LINES-1, COLS - 26, "char: ----");
    mvprintw(LINES-1, COLS - 26, "char: %c", info->contents[info->cy][info->cx]);
    move(info->wcy, info->cx);
	refresh();
	print_to_log("done updating cursor pos...");
}

void manage_input(char *file_name, WINDOW *win, unblind_info_t *info) {
	print_to_log("managing input...\n");
	reset_unblind_info_contents(info);
	char c = getch();
	if(info->m == FIND) {
		if(c == ENTER_KEY) {
			find_str(win, info);
			info->m = EDIT;
			memset(info->fstr, 0, sizeof(char) * MAX_CHARS_PER_LINE);
		} else if(c == BACKSPACE_KEY_0 || c == BACKSPACE_KEY_1 || c == BACKSPACE_KEY_2) {
			info->fstr[strlen(info->fstr)-1] = '\0';
			info->wcx--;
			strcpy(info->message, info->fstr);
			draw(win, info);
		} else if((c >= 'A' && c <= 'z')) {
			if(strlen(info->fstr)+1 == sizeof(char) * MAX_CHARS_PER_LINE) return; // this is very long shouldn't need to be any bigger
			info->fstr[strlen(info->fstr)] = c;
			info->wcx++;
			strcpy(info->message, info->fstr);
			draw(win, info);
		}
		return;
	}
	if(c == EOF) {
		return; // DON'T REDRAW SCREEN
	}

	int x;
	x = c;
	if(c == 27) {
		c = getch();
		if(c == 91) { // is not a ctrl arrow key
			c = getch();
			x = ARROW_KEY_MOD + c;
		} else if(c == 79) { // is a ctrl arrow key
			c = getch();
			x = CTRL_ARROW_KEY_MOD + c;
		}
	}

	switch(x) {
		case CTRL_DOWN_ARROW:
			for(int i = info->cy; i < MAX_LINES; i++) {
				if(info->contents[i][0] == '\0') {
					break;
				}
				move_cursor_down(win, info);
			}
			info->cx = strlen(info->contents[info->cy])-1;
			break;
		case CTRL_UP_ARROW:
			for(int i = info->cy; i != 0; i--) {
				if(info->contents[i] == NULL) {
					break;
				}
				move_cursor_up(win, info);
			}
			info->cx = strlen(info->contents[info->cy])-1;
			break;
		case CTRL_RIGHT_ARROW:
			for(int i = 0; i < (strlen(info->contents[info->cy]) - 1) - info->cx;) {
				move_cursor_right(win, info);
			}
			break;
		case CTRL_LEFT_ARROW:
			for(int i = 0; i < info->cx;) {
				if(info->contents[info->cy][info->cx-1] == TAB_KEY) {
					break;
				}
				move_cursor_left(win, info);
			}
			break;
		case DOWN_ARROW: // down arrow
			move_cursor_down(win, info);
			break;
		case UP_ARROW: // up arrow
			move_cursor_up(win, info);
			break;
		case LEFT_ARROW: // left arrow
			move_cursor_left(win, info);
			break;
		case RIGHT_ARROW: // right arrow
			move_cursor_right(win, info);
			break;
		case CTRL_F:
			info->m = FIND;
			memset(info->fstr, 0, sizeof(char) * MAX_CHARS_PER_LINE);
			memset(info->message, 0, MAX_CHARS_PER_LINE * sizeof(char));
			break;
		case CTRL_P:
			next_find_str(win, info);
			break;
		case CTRL_Q: // ctrl-q
			shutdown(info);
			break;
		case CTRL_S: // ctrl-s
			save_file(file_name, info);
			break;
		case BACKSPACE_KEY_0:
		case BACKSPACE_KEY_1:
		case BACKSPACE_KEY_2:
			backspace_action(win, info, 1);
			break;
		case TAB_KEY:
			tab_action(win, info, 1);
			break;
		case ENTER_KEY:
			enter_key_action(win, info, 1);
			break;
		case CTRL_D: // ctrl-d
			duplicate_line(win, info);
			break;
		case CTRL_X: // ctrl-x
			print_to_log("deleting line...\n");
			delete_line(win, info);
			print_to_log("Shifting up...\n");
			shift_up(win, info);
			break;
		case CTRL_Z: // ctrl-z
			if(info->ur_manager->stack_u->tail != NULL) {
				dll_node_t *node = (dll_node_t *) info->ur_manager->stack_u->tail;
				ur_node_t *ur_node = (ur_node_t *) node->value;
				if(ur_node->action == TYPE) {
					undo_type_char(win, info, node->x, node->y);
				} else if(ur_node->action == BACKSPACE) {
					undo_backspace(win, info, ur_node->c, node->x, node->y);
				} else if(ur_node->action == BACKSPACE_LAST_CHAR) {
					undo_last_backspace(win, info, ur_node->c, node->x, node->y);
				} else if(ur_node->action == TAB) {
					undo_tab(win, info, node->x, node->y);
				} else if(ur_node->action == ENTER) {
					undo_enter(win, info, node->y);
				}
			}
			// strcpy(info->message, (char *) linked_list_d_get(info->ur_manager->stack_u, 0)->value);
			// ur_action(win, info, (char *) info->ur_manager->stack_u->tail->value, info->ur_manager->stack_u->tail->x, info->ur_manager->stack_u->tail->y);
			//linked_list_d_pop(info->ur_manager->stack_u);
			break;
		default:
			type_char(c, info, 1);
			break;
	}

	draw(win, info);
}

void duplicate_line(WINDOW *win, unblind_info_t *info) {
	shift_down(win, info);
	strcpy(info->contents[info->cy], info->contents[info->cy-1]);
}

void delete_line(WINDOW *win, unblind_info_t *info) {
	int i = 0;
	while(info->contents[info->cy][i]) {
		move_to_left(info->contents[info->cy], i);
	}
	if(info->cy == 0 && info->contents[info->cy+1][0] == '\0') {
		info->contents[info->cy][0] = '\n';
		info->contents[info->cy][1] = '\0';
		info->cx = 0;
	} else if(info->contents[info->cy + 1][0] == '\0') {
		info->cy--;
		info->wcy--;
		unblind_scroll_check(win, info);
	}
}

void move_to_left(char *arr, int left) {
    for(int j = left; j < MAX_CHARS_PER_LINE; j++) {
        arr[j] = arr[j + 1];
    }
}

void shift_up(WINDOW *win, unblind_info_t *info) {
	if(info->cy == 0 && info->contents[info->cy+1][0] == '\0') { // at the top of the file, return
		return;
	}
	for(int i = info->cy+1; i < MAX_LINES; i++) {
		if(info->contents[i][0] == '\0') {
			break;
		}
		strcpy(info->contents[i-1], info->contents[i]);
		memset(info->contents[i], 0, MAX_CHARS_PER_LINE * sizeof(char));
		//strcpy(info->contents[i], "");
	}
	info->cx = strlen(info->contents[info->cy])-1;
}

void shift_down(WINDOW *win, unblind_info_t *info) {
	for(int i = MAX_LINES-1; i > info->cy; i--) {
		strcpy(info->contents[i], info->contents[i-1]);
	}
	info->cy++;
	info->wcy++;
	unblind_scroll_check(win, info);
}


int array_insert(char *a, int x, char c) {
	char par[255] = "";
	int j = 0;
	for(int i = x; i < strlen(a); i++) {
		par[j] = a[i];
		j++;
	}
	a[x] = c;
	a[x+1] = '\0';
	strcat(a, par);
	if(a[x] != '\0') return 1;
	return 0;
}

void print_to_log(const char *error) {
	//TODO remove this file later, kind of annoying
    FILE *fedit = fopen(".unblind_log.txt", "w+");
		if(strlen(error)) {
			fputs(error, fedit);
		} else if(error[0] == '\n') {
			fputs("\n", fedit);
		}
    fclose(fedit);
}


void reset_unblind_info_contents(unblind_info_t *info) {
	for(int j = 0; j < MAX_LINES-1; j++) {
		if(info->contents[j] == NULL) break;
		for(int i = strlen(info->contents[j]); i < MAX_CHARS_PER_LINE; ++i) {
			info->contents[j][i] = '\0';
		}
	}
}

void setup_unblind_info(unblind_info_t *info) {
	info->cx = 0;
	info->cy = 0;
	info->wcy = 0;
	info->wcx = 0;
	info->scroll_offset = 0;

	info->ur_manager = (undo_redo_manager_t *) malloc(sizeof(undo_redo_manager_t));
	setup_unblind_ur_manager(info->ur_manager);

	info->message = (char *)malloc(MAX_CHARS_PER_LINE * sizeof(char));
	memset(info->message, 0, MAX_CHARS_PER_LINE * sizeof(char));
	info->find = (d_linked_list_t *)malloc(sizeof(d_linked_list_t));
	info->fstr = (char *)malloc(sizeof(char) * MAX_CHARS_PER_LINE);
	memset(info->fstr, 0, sizeof(char) * MAX_CHARS_PER_LINE);
	info->contents = (char **)malloc(MAX_LINES * sizeof(char *));
	for(int i = 0; i < MAX_LINES; i++) {
		info->contents[i] = (char *)malloc(MAX_CHARS_PER_LINE * sizeof(char));
		memset(info->contents[i], 0, MAX_CHARS_PER_LINE * sizeof(char));
	}
}

void unblind_info_free(unblind_info_t *info) {
	for(int i = 0; i < MAX_LINES; i++) {
		free(info->contents[i]);
	}
	free(info->contents);
	free(info->message);
	free(info->fstr);
	free(info->find);
	free(info->ur_manager);
	free(info);
}

void shutdown(unblind_info_t *info) {
	endwin();
	unblind_info_free(info);
	exit(0);
}
