#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "double_linked_list.h"
#include "unblind.h"
#include "actions.h"

//TODO find command doesn't move the cursor to the message area

void jump_to_start(WINDOW *win, unblind_info_t *info) {
    info->scroll_offset = 0;
    info->scrollX_offset = 0;
    info->wcy = 0;
    info->cy = 0;
    info->cx = 0;
    info->wcx = 0;
    update_cursor_pos(win, info);
}

void jump_to_end(WINDOW *win, unblind_info_t *info) {
    int i = 0;
    
    for(; i < MAX_LINES; i++) {
        if(info->contents[i][0] == '\0') break;
    }
    i--;
    int j = strlen(info->contents[i])-1;
    int scrollPos_y = i-(LINES_PER_WINDOW-7);
    int scrollPos_x = j-(CHARS_PER_LINE_PER_WINDOW-SCROLLX_THRESHOLD+1);
    if(j <= CHARS_PER_LINE_PER_WINDOW-SCROLLX_THRESHOLD+1) 
        info->scrollX_offset = 0;
    else
        info->scrollX_offset = scrollPos_x;
    
    info->cx = j;
    if(j- 11 > CHARS_PER_LINE_PER_WINDOW) {
        info->wcx = CHARS_PER_LINE_PER_WINDOW-SCROLLX_THRESHOLD+1;
    } else {
        info->wcx = j;
    }
    
    if(i <= LINES_PER_WINDOW-7) 
        info->scroll_offset = 0;
    else
        info->scroll_offset = scrollPos_y;
    info->cy = i;
    if(i-7 > LINES_PER_WINDOW) {
        info->wcy = LINES_PER_WINDOW-7;
    } else {
        info->wcy = i;
    }
    
    
    update_cursor_pos(win, info);
}

void move_cursor_up(WINDOW *win, unblind_info_t *info) {
	 if(!(info->cy-1 <= -1)) {
		info->cy--;
		info->wcy--;
//         unblind_scroll_hor_calc(win, info);
        while(current_character(info) == TAB_KEY) {
            info->cx++;
            info->wcx++;
        }
    }
    if(info->cx > strlen(current_line(info))-1) {
        info->cx = strlen(current_line(info))-1;
        info->wcx = strlen(current_line(info))-1;
    } else if(current_line(info)[0] == '\n') {
        info->cx = 0;
        info->wcx = 0;
    }
    unblind_scroll_check(win, info);
	update_cursor_pos(win, info);
}

void move_cursor_down(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy+1][info->cx] == TAB_KEY) {
		info->cy++;
		info->wcy++;
		while(current_character(info) == '\0') {
			move_cursor_left(win, info);
		}
		while(current_character(info) == TAB_KEY) {
			info->cx++;
            info->wcx++;
		}
	} else if(info->cx <= strlen(next_line(info)) && info->contents[info->cy+1][0] != '\0') {
		info->cy++;
		info->wcy++;
	} else if(next_line(info)[0] != '\0') {
		info->cy++;
		info->wcy++;
		info->cx = strlen(current_line(info))-1;
		info->wcx = strlen(current_line(info))-1;
	}
	unblind_scroll_check(win, info);
	update_cursor_pos(win, info);
}

void move_cursor_left(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy][info->cx-1] == TAB_KEY) {
        info->cx--;
        info->wcx--;
        if(info->cx == 0) {
            move_cursor_up(win, info);
        }
		while(current_character(info) == TAB_KEY) {
			info->cx--;
            info->wcx--;
            if(info->cx == 0) {
                move_cursor_up(win, info);
            }
		}
	} else if(info->wcx-1 != -1) {
		info->wcx--;
		info->cx--;
	} else if(info->wcx-1 == -1 && !(info->cy-1 <= -1)) {
		--info->cy;
		info->wcx = strlen(current_line(info))-1;
		info->cx = strlen(current_line(info))-1;
		info->wcy--;
        unblind_scroll_hor_calc(win, info);
	}
	unblind_scroll_check(win, info);
	update_cursor_pos(win, info);
}

void move_cursor_right(WINDOW *win, unblind_info_t *info) {
	if(current_character(info) == TAB_KEY) {
		while(current_character(info) == TAB_KEY) {
			info->cx++;
            info->wcx++;
		}
	} else if(current_character(info) != '\n' && current_character(info) != '\0') {
		info->cx++;
		info->wcx++;
	} else if((next_line(info)[0] || next_line(info)[0] == '\n') && !(info->cy+1 >= MAX_LINES)) {
		info->cx = 0;
		info->wcx = 0;
        info->scrollX_offset = 0;
		info->cy++;
		info->wcy++;
		unblind_scroll_check(win, info);
		if(current_character(info) == TAB_KEY) {
			info->cx++;
			info->wcx++;
			while(current_character(info) == TAB_KEY) {
				info->cx++;
				info->wcx++;
			}
		}
	}
	unblind_scroll_check(win, info);
	update_cursor_pos(win, info);
}

/**
* Finds a word and moves the cursor to the first occurence of the word
* to get the next word recall the function
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
	if(tmp == NULL) {
		strcpy(info->message, "No results found!");
		return;
	}
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
	while(info->cx != tmp->x) {
		if(tmp->x > info->cx) {
            info->cx++;
            info->wcx++;
        } else if(tmp->x < info->cx) {
            info->cx--;
            info->wcx--;
        }
		unblind_scroll_check(win, info);
	}
    update_cursor_pos(win, info);
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
	
	while(info->cx != tmp->x) {
		if(tmp->x > info->cx) {
            info->cx++;
            info->wcx++;
        } else if(tmp->x < info->cx) {
            info->cx--;
            info->wcx--;
        }
		unblind_scroll_check(win, info);
	}
	
	unblind_scroll_check(win, info);
}

void backspace_action(WINDOW *win, unblind_info_t *info, int add_to_ur_manager) {
	if(info->cx <= 0 && info->cy <= 0) return;
	char del;
	if(info->cx <= 0 && info->cy > 0) {
		if(info->cy == 0) {
			unblind_scroll_up(win, info);
		}
		info->cy--;
		int len = strlen(current_line(info));
		info->cx = len;
		info->wcx = len;
		del = current_character(info);
		info->cx--;
		info->wcx--;
		mvwdelch(win, info->cy, info->cx);
		info->contents[info->cy][info->cx] = '\0';
		strcat(info->contents[info->cy], info->contents[info->cy+1]);
		info->cy++;
		delete_line(win, info);
		if(next_line(info)[0] == '\0') { // undo movement caused by delete line if the last line is deleted
			info->cy++;
			info->wcy++;
		}
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
        unblind_scroll_hor_calc(win, info);
		if(add_to_ur_manager) {
			ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
			char *del1 = &del;
			node->c = (char *) malloc(sizeof(char)); // won't be used
			node->c = strdup(del1);
			node->action = BACKSPACE_LAST_CHAR;
			linked_list_d_add(info->ur_manager->stack_u, (void *) node, info->cx, info->cy);
		}
		unblind_scroll_check(win, info);
	} else {
		if(prev_character(info) == 9) {
			for(int i = 0; i < 4; i++) {
				info->cx--;
				info->wcx--;
				del = current_character(info);
				move_to_left(info->contents[info->cy], info->cx);
			}
		} else {
			info->cx--;
			info->wcx--;
			del = current_character(info);
			move_to_left(info->contents[info->cy], info->cx);
			update_cursor_pos(win, info);
		}
		if(add_to_ur_manager) {
			ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
			char *del1 = &del;
			node->c = (char *) malloc(sizeof(char)); // won't be used
			node->c = strdup(del1);
			node->action = BACKSPACE;
			linked_list_d_add(info->ur_manager->stack_u, (void *) node, info->cx, info->cy);
		}
	}
     unblind_scroll_check(win, info);

	// char *del1 = &del;
	// char *del2 = (char *) malloc(sizeof(char));
	// del2 = strdup(del1);
	// linked_list_d_add(info->ur_manager->stack_u, (void *) del2, info->cx, info->cy);
	// strcpy(info->message, (char *) linked_list_d_get(info->ur_manager->stack_u, 0)->value);
}

void enter_key_action(WINDOW *win, unblind_info_t *info, int add_to_ur_manager) {
	//int x = info->cx;
	//int y = info->cy;
	char c = '\n';
	if(strlen(current_line(info)) == 0) {
		array_insert(info->contents[info->cy], info->cx, c, MAX_CHARS_PER_LINE);
		info->cy++;
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
        
        int i = strlen(info->contents[info->cy])-1;
        int scrollPos_x = i-(CHARS_PER_LINE_PER_WINDOW-SCROLLX_THRESHOLD+1);
        if(i <= CHARS_PER_LINE_PER_WINDOW-SCROLLX_THRESHOLD+1) 
            info->scrollX_offset = 0;
        else
            info->scrollX_offset = scrollPos_x;
        
        info->cx = j;
        if(j- 11 > CHARS_PER_LINE_PER_WINDOW) {
            info->wcx = CHARS_PER_LINE_PER_WINDOW-SCROLLX_THRESHOLD+1;
        } else {
            info->wcx = j;
        }
        
		info->cx = 0;
		info->wcx = 0;
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
		free(partition);
	}
	unblind_scroll_check(win, info);
	info->wcy++;
	update_cursor_pos(win, info);
	if(add_to_ur_manager == 1) {
		ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
		node->c = " "; // won't be used
		node->action = ENTER;
		linked_list_d_add(info->ur_manager->stack_u, (void *) node, info->cx,info->cy);
	}
}

void save_file(char *file_name, unblind_info_t *info) {
	write_contents_to_file(file_name, info);
	strcpy(info->message, "");
	strcat(info->message, "Saved ");
	strcat(info->message, file_name);
}

void type_char(WINDOW *win, char c, unblind_info_t *info, int add_to_ur_manager) {
	int x = info->cx;
	int y = info->cy;
	if(c != EOF && c != '\0') {
        if(!iscntrl(c)) { //TODO this if doesn't work
            switch(c) {
                case ')':
                    if(next_character(info) == ')') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
                case ']':
                    if(next_character(info) == ']') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
                case '>':
                    if(next_character(info) == '>') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
                case '\'':
        		case '\"':
                    if(next_character(info) == '\'' || next_character(info) == '\"') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
                case '}':
                    if(next_character(info) == '}') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
            }
			array_insert(info->contents[info->cy], info->cx, c, MAX_CHARS_PER_LINE);
			info->cx++;
			info->wcx++;
            unblind_scroll_check(win, info);
			// auto completion for ()'s and such
        	switch(c) {
        		case '(':
        			array_insert(info->contents[info->cy], info->cx, ')', MAX_CHARS_PER_LINE);
        			break;
        		case '[':
        			array_insert(info->contents[info->cy], info->cx, ']', MAX_CHARS_PER_LINE);
        			break;
        		case '{':
        			array_insert(info->contents[info->cy], info->cx, '}', MAX_CHARS_PER_LINE);
        			break;
        		case '\'':
        		case '\"':
        			array_insert(info->contents[info->cy], info->cx, c, MAX_CHARS_PER_LINE);
	       			break;
        		case '<':
        			array_insert(info->contents[info->cy], info->cx, '>', MAX_CHARS_PER_LINE);
        			break;
                
        	}
        }
        strcpy(info->message, "");
		if(add_to_ur_manager == 1) {
			ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
			node->c = " "; // won't be used
			node->action = TYPE;
			linked_list_d_add(info->ur_manager->stack_u, (void *) node, x, y);
		}
	} else {
		return; // DO NOT REDDRAW SCREEN
	}
}

void tab_action(WINDOW *win, unblind_info_t *info, int add_to_ur_manager) {
	int x = info->cx;
	int y = info->cy;
	for(int i = 0; i < 4; i++) {
		array_insert(info->contents[info->cy], info->cx++, TAB_KEY, MAX_CHARS_PER_LINE);
		info->wcx++;
        unblind_scroll_hor_calc(win, info);
	}
	if(add_to_ur_manager) {
		ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
		node->c = " "; // won't be used
		node->action = TAB;
		linked_list_d_add(info->ur_manager->stack_u, (void *) node, x, y);
	}
}

char current_character(unblind_info_t *info) {
	return info->contents[info->cy][info->cx];
}

char next_character(unblind_info_t *info) {
	return info->contents[info->cy][info->cx + 1];
}

char prev_character(unblind_info_t *info) {
	return info->contents[info->cy][info->cx - 1];
}

char *current_line(unblind_info_t *info) {
	return info->contents[info->cy];
}

char *next_line(unblind_info_t *info) {
	return info->contents[info->cy + 1];
}

char *prev_line(unblind_info_t *info) {
	return info->contents[info->cy - 1];
}

void undo_type_char(WINDOW *win, unblind_info_t *info, int x, int y) {
	move_to_left(info->contents[y], x);
	info->cx = x;
	while(y != info->cy) {
		if(info->wcy < y) {
			info->cy++;
			info->wcy++;
            unblind_scroll_hor_calc(win, info);
	 	} else if(info->cy > y) {
			info->cy--;
			info->wcy--;
            unblind_scroll_hor_calc(win, info);
	 	}
		unblind_scroll_check(win, info);
	}
	linked_list_d_pop(info->ur_manager->stack_u);
}

void undo_backspace(WINDOW *win, unblind_info_t *info, char *c, int x, int y) {
	strcpy(info->message, c);
	info->cx = x;
	while(y != info->cy) {
		if(info->wcy < y) {
			info->cy++;
			info->wcy++;
            unblind_scroll_hor_calc(win, info);
	 	} else if(info->cy > y) {
			info->cy--;
			info->wcy--;
            unblind_scroll_hor_calc(win, info);
	 	}
		unblind_scroll_check(win, info);
	}
	if(*c == TAB_KEY) {
		tab_action(win, info, 0);
		strcpy(info->message, "TAB"); //TODO remove constant	
	} else {
		type_char(win, *c, info, 0);
	}
	linked_list_d_pop(info->ur_manager->stack_u);
}

void undo_last_backspace(WINDOW *win, unblind_info_t *info, char *c, int x, int y) {
	strcpy(info->message, c);
	info->cx = x;
	info->cy = y;
	enter_key_action(win, info, 0);
	info->cx = x;
	info->cy = y;
	info->wcy = y;
	info->wcx = x;
    unblind_scroll_hor_calc(win, info);
	linked_list_d_pop(info->ur_manager->stack_u);
}

void undo_enter(WINDOW *win, unblind_info_t *info, int y) {
	while(y != info->cy) {
		if(info->wcy < y) {
			info->cy++;
			info->wcy++;
            unblind_scroll_hor_calc(win, info);
	 	} else if(info->cy > y) {
			info->cy--;
			info->wcy--;
            unblind_scroll_hor_calc(win, info);
	 	}
		unblind_scroll_check(win, info);
	}
	info->cx = 0;
	info->wcx = 0;
	unblind_scroll_check(win, info);
	backspace_action(win, info, 0);
	linked_list_d_pop(info->ur_manager->stack_u);
}

void undo_tab(WINDOW *win, unblind_info_t *info, int x, int y) {
	int i = 0;
	while(i < TAB_SIZE) {
		move_to_left(info->contents[y], x);
		info->cx = x;
		info->wcx = x;
		while(y != info->cy) {
			if(info->wcy < y) {
				info->cy++;
				info->wcy++;
                unblind_scroll_hor_calc(win, info);
		 	} else if(info->cy > y) {
				info->cy--;
				info->wcy--;
                unblind_scroll_hor_calc(win, info);
		 	}
			unblind_scroll_check(win, info);
		}
		i++;
	}
	linked_list_d_pop(info->ur_manager->stack_u);
}
