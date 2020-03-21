#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "unblind.h"
#include "actions.h"

void move_cursor_up(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy-1][info->cx] == 9) {
		info->cx++;
		info->cy--;
		info->wcy--;
		while(info->contents[info->cy][info->cx] == 9) {
			info->cx++;
		}
	} else if(!(info->cy-1 <= -1)) {
		info->cy--;
		info->wcy--;
		if(info->cx > strlen(info->contents[info->cy])-1) {
			info->cx = strlen(info->contents[info->cy])-1;
		} else if(info->contents[info->cy][0] == '\n') {
			info->cx = 0;
		}
		if(info->wcy <= 6 && info->scroll_offset > 0) {
			info->wcy++;
			unblind_scroll_up(win, info);
		}
	}
}

void move_cursor_down(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy+1][info->cx] == 9) {
		info->cx++;
		info->cy++;
		info->wcy++;
		while(info->contents[info->cy][info->cx] == 9) {
			info->cx++;
		}
	} else if(info->cx <= strlen(info->contents[info->cy+1]) && info->contents[info->cy+1][0] != '\0') {
		info->cy++;
		info->wcy++;
	} else if(info->contents[info->cy+1][0] != '\0') {
		info->cy++;
		info->wcy++;
		info->cx = strlen(info->contents[info->cy])-1;
	}
	if(LINES-6 <= info->wcy && info->contents[info->wcy-1][0] != '\0') {
		info->wcy--;
		unblind_scroll_down(win, info);
	}
}

void move_cursor_left(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy][info->cx-1] == 9) {
		info->cx -= 4;
		if(info->cx == 0) {
			move_cursor_up(win, info);
		}
	} else if(info->contents[info->cy][info->cx-1] && info->cx-1 != -1) {
		info->cx--;
	} else if(info->cx-1 == -1 && !(info->cy-1 <= -1)) {
		--info->cy;
		info->cx = strlen(info->contents[info->cy])-1;	
		info->wcy--;
	}
	if(info->cy == 0 && info->cx == 0) {
		unblind_scroll_up(win, info);
	}
}

void move_cursor_right(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy][info->cx] == 9) {
		info->cx += 4;
	} else if(info->contents[info->cy][info->cx] != '\n' && info->contents[info->cy][info->cx] != '\0') {
		info->cx++;
	} else if((info->contents[info->cy+1][0] || info->contents[info->cy+1][0] == '\n') && !(info->cy+1 >= MAX_LINES)) {
		info->cx = 0;
		if(info->contents[info->cy+1][info->cx] == 9) {
			info->cx++;
			info->cy++;
			info->wcy++;
			while(info->contents[info->cy][info->cx] == 9) {
				info->cx++;
			}
		} else {
			info->cy++;
			info->wcy++;
		}
		
	}
	if(LINES_PER_WINDOW-1 == info->cy-info->scroll_offset && info->cx == strlen(info->contents[info->cy])) {
			unblind_scroll_down(win, info);
	}
}

void backspace_action(WINDOW *win, unblind_info_t *info) {
	if(info->cx <= 0 && info->cy <= 0) return;
	if(info->cx <= 0 && info->cy > 0) {
		if(info->cy == 0) {
			unblind_scroll_up(win, info);
		}
		info->cy--;
		int len = strlen(info->contents[info->cy]);
		info->cx = len;
		info->cx--;
		mvwdelch(win, info->cy, info->cx);
		info->contents[info->cy][info->cx] = '\0';
		strcat(info->contents[info->cy], info->contents[info->cy+1]);
		delete_line(win, (info->cy + 1), info);
		strcpy(info->contents[info->cy+1], "\n");
		info->cy++;
		
		for(int k = info->cy; k < MAX_LINES-1; k++) {
			strcpy(info->contents[k], info->contents[k+1]);
		}
		
		if(info->wcy <= 6 && info->scroll_offset > 0) {
			info->wcy++;
			unblind_scroll_up(win, info);
		}
		info->cy--;
		info->wcy--;
	} else {
		if(info->contents[info->cy][info->cx-1] == 9) {
			for(int i = 0; i < 4; i++) {
				info->cx--;
				move_to_left(info->contents[info->cy], info->cx);
			}
		} else {
			info->cx--;
			move_to_left(info->contents[info->cy], info->cx);
			update_cursor_pos(win, info);
		}
	}
}

void enter_key_action(WINDOW *win, unblind_info_t *info) {
	char c = '\n';
	if(strlen(info->contents[info->cy]) == 0) {
		if(!array_insert(info->contents[info->cy], info->cx, c)) {
			info->contents[info->cy][info->cx] = c;
			info->cy++;
		}
	} else {
		//char partition[MAX_CHARS_PER_LINE] = "";
		char *partition = (char *)malloc(MAX_CHARS_PER_LINE * sizeof(char));
		memset(partition, 0, MAX_CHARS_PER_LINE * sizeof(char));
		
		int j = 0;
		for(int i = info->cx; i <= strlen(info->contents[info->cy]); i++) {
			partition[j] = info->contents[info->cy][i];
			j++;
		}
		int new_length = strlen(info->contents[info->cy]) - strlen(partition);
		info->contents[info->cy][new_length] = '\n';
		info->contents[info->cy][new_length+1] = '\0';
		info->cy++;
		info->cx = 0;
		for(int k = MAX_LINES-1; k > info->cy; k--) {
			if(info->contents[k-1] == NULL) {
				//info->contents[k] = NULL;
				memset(info->contents[k], 0, MAX_CHARS_PER_LINE * sizeof(char));
			} else {
				strcpy(info->contents[k], info->contents[k-1]);
				//info->contents[k] = strdup(info->contents[k-1]);
			}
		}
		strcpy(info->contents[info->cy], partition);
	}
	if(LINES-6 <= info->wcy && info->contents[info->wcy-1][0] != '\0') {
		info->wcy--;
		unblind_scroll_down(win, info);
	}
	info->wcy++;
	//cy--;
}

void save_file(char *file_name, unblind_info_t *info) {
	write_contents_to_file(file_name, info);
	strcat(info->message, "Saved ");
	strcat(info->message, file_name);
}

void type_char(char c, unblind_info_t *info) {
	if(c != EOF) {
        if(!iscntrl(c)) {
			array_insert(info->contents[info->cy], info->cx, c);
			info->cx++;
			// auto completion for ()'s and such
        	switch(c) {
        		case '(':
        			array_insert(info->contents[info->cy], info->cx, ')');
        			break;
        		case '[':
        			array_insert(info->contents[info->cy], info->cx, ']');
        			break;
        		case '{':
        			array_insert(info->contents[info->cy], info->cx, '}');
        			break;
        		case '\'':
        		case '\"':
        			array_insert(info->contents[info->cy], info->cx, c);
	       			break;
        		case '<':
        			array_insert(info->contents[info->cy], info->cx, '>');
        			break;
        	}
        }
        strcpy(info->message, "");
	} else {
		return; // DO NOT REDDRAW SCREEN
	}
}

void tab_action(WINDOW *win, unblind_info_t *info) {
	for(int i = 0; i < 4; i++) {
		array_insert(info->contents[info->cy], info->cx++, 9);
	}
}
