#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "unblind.h"
#include "actions.h"

void move_cursor_up(WINDOW *win, unblind_info_t *info) {
	 if(!(info->cy-1 <= -1)) {
		info->cy--;
		info->wcy--;
		if(info->contents[info->cy][info->cx] == TAB_KEY) {
				info->cx++;
				while(info->contents[info->cy][info->cx] == TAB_KEY) {
					info->cx++;
				}
		} else if(info->cx > strlen(info->contents[info->cy])-1) {
			info->cx = strlen(info->contents[info->cy])-1;
		} else if(info->contents[info->cy][0] == '\n') {
			info->cx = 0;
		}
		unblind_scroll_check(win, info);
	}
}

void move_cursor_down(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy+1][info->cx] == TAB_KEY) {
		info->cx++;
		info->cy++;
		info->wcy++;
		while(info->contents[info->cy][info->cx] == TAB_KEY) {
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
	unblind_scroll_check(win, info);
}

void move_cursor_left(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy][info->cx-1] == TAB_KEY) {
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
	if(info->cx == 0) {
		unblind_scroll_check(win, info);
	}
}

void move_cursor_right(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy][info->cx] == TAB_KEY) {
		info->cx += 4;
	} else if(info->contents[info->cy][info->cx] != '\n' && info->contents[info->cy][info->cx] != '\0') {
		info->cx++;
	} else if((info->contents[info->cy+1][0] || info->contents[info->cy+1][0] == '\n') && !(info->cy+1 >= MAX_LINES)) {
		info->cx = 0;
		info->cy++;
		info->wcy++;
		unblind_scroll_check(win, info);
		if(info->contents[info->cy][info->cx] == TAB_KEY) {
			info->cx++;
			while(info->contents[info->cy][info->cx] == TAB_KEY) {
				info->cx++;
			}
		}
	}
}

/**
* Finds a word and moves the cursor to the first occurence of the word
* to get the next word call next_find_str function
*/
void find_str(WINDOW *win, unblind_info_t *info) {
	info->find = linked_list_d_create();
	// strcpy(str, info->fstr);
	int state = 0;
	for(int j = 0; info->contents[j][0] != '\0'; j++) {
		for(int i = 0; i < strlen(info->contents[j]); i++) {
			if(info->fstr[state] == info->contents[j][i]) {
				state++;
			} else {
				state = 0;
			}
			if(state == strlen(info->fstr)) {
				// info->value = (void *) str;
				linked_list_d_add(info->find, (void *) info->fstr, (i+1)-strlen(info->fstr), j);
				state = 0;
			}

		}
	}
	dll_node_t *tmp = linked_list_d_get(info->find, info->find->curr);
	if(tmp == NULL) { //TODO don't make this shutdown
		strcpy(info->message, "No results found!");
		return;
	}
	while(info->cx != tmp->x) {
		if(tmp->x > info->cx) info->cx++;
		if(tmp->x < info->cx) info->cx--;
		unblind_scroll_check(win, info);
	}
	//info->cx = tmp->x;
	while(info->cy != tmp->y) {
		if(tmp->y > info->cy) {
			info->cy++;
			info->wcy++;
	 	}
		if(tmp->y < info->cy) {
			info->cy--;
			info->wcy--;
	 	}
		unblind_scroll_check(win, info);
	}
	// info->cx = tmp->x;
	// info->cy = tmp->y;
	// info->wcy = tmp->y;
	// info->wcx = tmp->x;
	unblind_scroll_check(win, info);
	update_cursor_pos(win, info);
}

void next_find_str(WINDOW *win, unblind_info_t *info) {
	dll_node_t *tmp = linked_list_d_get(info->find, info->find->curr);
	if(tmp == NULL) {
		if(info->find->head != NULL) {
			info->find->curr = 0;
			next_find_str(win, info);
			return;
		}
		strcpy(info->message, "No results found!");
		return;
	}
	while(info->cx != tmp->x) {
		if(tmp->x > info->cx) info->cx++;
		if(tmp->x < info->cx) info->cx--;
		unblind_scroll_check(win, info);
	}
	//info->cx = tmp->x;
	while(info->cy != tmp->y) {
		if(tmp->y > info->cy) {
			info->cy++;
			info->wcy++;
	 	}
		if(tmp->y < info->cy) {
			info->cy--;
			info->wcy--;
	 	}
		unblind_scroll_check(win, info);
	}
	//info->cy = tmp->y;
	//info->wcy = tmp->y;
	// info->wcx = tmp->x;
	unblind_scroll_check(win, info);
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
		info->cy++;
		delete_line(win, info);
		strcpy(info->contents[info->cy], "\n");

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
	unblind_scroll_check(win, info);
	info->wcy++;
	/*if(LINES-6 <= info->wcy && info->contents[info->wcy-1][0] != '\0') {
		info->wcy--;
		unblind_scroll_down(win, info);
	}*/
	//info->wcy++;
	//cy--;
}

void save_file(char *file_name, unblind_info_t *info) {
	write_contents_to_file(file_name, info);
	strcpy(info->message, "");
	strcat(info->message, "Saved ");
	strcat(info->message, file_name);
}

void type_char(char c, unblind_info_t *info) {
	if(c != EOF) {
        if(!iscntrl(c)) { //TODO this if doesn't work
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
