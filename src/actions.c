#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "double_linked_list.h"
#include "unblind.h"
#include "actions.h"
#include "mainframe.h"

void jump_to_start(WINDOW *win, unblind_info_t *info) {
    info->wcy = 0;
    info->cy = 0;
    info->cx = 0;
    info->wcx = 0;
    unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
    update_cursor_pos(win, info);
}

void jump_to_end(WINDOW *win, unblind_info_t *info) {
    int i = 0;
    
    for(; i < MAX_LINES; i++) {
        if(info->contents[i][0] == '\0') break;
    }
    i--;
    info->cy = i;
    if(current_line(info)[strlen(current_line(info))-1] == '\n') info->cx = strlen(current_line(info))-1;
    else info->cx = strlen(current_line(info));
    unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
    
    update_cursor_pos(win, info);
}

void jump_to_line(WINDOW *win, unblind_info_t *info, int line) {
    if(info->contents[line][0] == '\0') {
        strcpy(info->message, "Line does not exist!"); // TODO remove string literal
        return;
    } else {
        info->cy = line;
        info->cx = 0;
        unblind_scroll_hor_calc(win, info);
        unblind_scroll_vert_calc(win, info);
    }
}

void move_cursor_up(WINDOW *win, unblind_info_t *info) {
	 if(!(info->cy-1 <= -1)) {
		info->cy--;
		info->wcy--;
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
    unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
}

void move_cursor_down(WINDOW *win, unblind_info_t *info) {
	if(next_line(info)[info->cx] == TAB_KEY) {
		info->cy++;
		info->wcy++;
		while(current_character(info) == '\0') {
			move_cursor_left(win, info);
		}
		while(current_character(info) == TAB_KEY) {
			info->cx++;
            info->wcx++;
		}
	} else if(info->cx <= (strlen(next_line(info))-1) && next_line(info)[0] != '\0') {
		info->cy++;
		info->wcy++;
	} else if(next_line(info) && next_line(info)[0] != '\0') {
		info->cy++;
		info->wcy++;
		info->cx = strlen(current_line(info))-1;
		info->wcx = strlen(current_line(info))-1;
	}
	unblind_scroll_hor_calc(win, info);
	unblind_scroll_vert_calc(win, info);
}

void move_cursor_left(WINDOW *win, unblind_info_t *info) {
	if(info->contents[info->cy][info->cx-1] == TAB_KEY) {
        info->cx--;
        if(info->cx == 0) {
            move_cursor_up(win, info);
        }
        if(current_character(info) == TAB_KEY) {
            info->cx-=TAB_SIZE;
            if(info->cx < 0) {
                info->cy--;
                info->cx = strlen(info->contents[info->cy])-1;
            }
        }
	} else if(info->cx-1 != -1) {
		info->cx--;
	} else if(info->cx-1 == -1 && !(info->cy-1 <= -1)) {
		--info->cy;
		info->cx = strlen(current_line(info))-1;
		info->wcy--;
	}
	unblind_scroll_vert_calc(win, info);
	unblind_scroll_hor_calc(win, info);
}

void move_cursor_right(WINDOW *win, unblind_info_t *info) {
	if(current_character(info) == TAB_KEY) {
        info->cx++;
        if((info->cx) == 0)  {
            info->cx += TAB_SIZE+1;
        } else {
            info->cx+=TAB_SIZE;
        }
	} else if(current_character(info) != '\n' && current_character(info) != '\0') {
		info->cx++;
	} else if(strlen(next_line(info)) > 0) {
		info->cy++;
        info->cx = 0;
	}
	unblind_scroll_vert_calc(win, info);
	unblind_scroll_hor_calc(win, info);
}

int hash_prime = 301; //TODO move this to top

int hash(char *str) {
	int sum = 0;
	for(int i = 0; i < strlen(str); i++) {
		sum += (str[i] * hash_prime);
	}
	return sum;
}

/**
* Finds a word and moves the cursor to the first occurence of the word
* to get the next word recall the function
*/
void find_str(WINDOW *win, unblind_info_t *info) {
	dll_node_t *tmp = NULL;
	if(info->find == NULL) {
		info->find = linked_list_d_create();

		int find = hash(info->fstr);
		int Fsize = strlen(info->fstr);
        for(int j = 0; j < MAX_LINES; j++) {
			if(Fsize == 1) {
                for(int i = 0; i <= info->size[j]; i++) {
                    if(info->contents[j][i] == '\0') break;
                    if(info->fstr[0] == info->contents[j][i]) linked_list_d_add(info->find, (void *) info->fstr, i, j);
                }
            } else {
                int Jsize = strlen(info->contents[j]);
                for(int i = 0; i <= Jsize && Jsize >= Fsize; i++) {
                    char *newStr = malloc(sizeof(char) * Fsize+1);
                    memset(newStr, 0, Fsize+1);
                    strncpy(newStr, info->contents[j]+i, Fsize);
                    int look = hash(newStr);
                    if(look == find) {
                        if(strcmp(newStr, info->fstr) == 0) {
                            linked_list_d_add(info->find, (void *) info->fstr, i, j);
                            
                        }
                    }
                    free(newStr);
                }
            }
		}
		tmp = linked_list_d_get(info->find, info->find->curr);
		if(tmp == NULL) {
			strcpy(info->message, "No results found!");
            unblind_scroll_hor_calc(win, info);
            unblind_scroll_vert_calc(win, info);
			return;
		}
	} else {
		tmp = linked_list_d_get(info->find, info->find->curr);
		if(tmp == NULL) {
			info->find->curr = 0;
			tmp = linked_list_d_get(info->find, info->find->curr);
		}
	}
    if(tmp != NULL) {
        info->cx = tmp->x;
        info->cy = tmp->y;
        unblind_scroll_hor_calc(win, info);
        unblind_scroll_vert_calc(win, info);
    }
}

void next_find_str(WINDOW *win, unblind_info_t *info) {
	dll_node_t *tmp = linked_list_d_get(info->find, info->find->curr);
	if(tmp == NULL) {
        find_str(win, info);
		return;
	}
	info->cx = tmp->x;
    info->cy = tmp->y;
	unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
	int find = hash(info->fstr);
	for(int j = tmp->y; j < MAX_LINES-1; j++) {
		for(int i = tmp->x; i < strlen(info->contents[j]) && strlen(info->contents[j]) >= strlen(info->fstr); i++) {
			char *newStr = malloc(sizeof(char) * strlen(info->fstr)+1);
			strncpy(newStr, info->contents[j]+i, strlen(info->fstr));
			newStr[strlen(info->fstr)+1] = '\0';
			
			int look = hash(newStr);
			if(look == find) {
				if(strcmp(newStr, info->fstr) == 0) {
					linked_list_d_add(info->find, (void *) info->fstr,  i, j);
					return;
				}
			}
			free(newStr);
		}
	}
	strcpy(info->message, "CTRL-P again to loop find!");
}

void backspace_action(WINDOW *win, unblind_info_t *info, int add_to_ur_manager) {
	if(info->cx <= 0 && info->cy <= 0) return;
	char del;
	if(info->cx <= 0 && info->cy > 0) {
		info->cy--;
		int len = strlen(current_line(info));
		info->cx = len;
		del = current_character(info);
		info->cx--;
		info->contents[info->cy][info->cx] = '\0';
		
		int new_length = len + strlen(next_line(info));
        
        info->size[info->cy] = MAX_CHARS_PER_LINE;
        while(info->size[info->cy] < new_length) info->size[info->cy] *= 2;
        info->contents[info->cy] = realloc(info->contents[info->cy], info->size[info->cy] * sizeof(char));
		
        strcat(info->contents[info->cy], info->contents[info->cy+1]);
        info->cy++;
        if(next_line(info)[0] == '\0') { // counters delete_line movement if needed
            info->cy++;
        }
		delete_line(win, info, 0);
		info->cy--;
        info->cx = len-1;
		if(add_to_ur_manager) {
			ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
			char *del1 = &del;
			node->c = (char *) malloc(sizeof(char)); // won't be used
			node->c = strdup(del1);
			node->action = BACKSPACE_LAST_CHAR;
			linked_list_d_add(info->ur_manager->stack_u, (void *) node, info->cx, info->cy);
		}
	} else {
		if(prev_character(info) == TAB_KEY) {
			for(int i = 0; i < 5; i++) {
				info->cx--;
				del = current_character(info);
				move_to_left(info->contents[info->cy], info->cx, strlen(info->contents[info->cy]));
			}
		} else if((prev_character(info) == '(' && current_character(info) == ')') ||
            (prev_character(info) == '"' && current_character(info) == '"') ||
            (prev_character(info) == '\'' && current_character(info) == '\'') ||
            (prev_character(info) == '{' && current_character(info) == '}') ||
            (prev_character(info) == '[' && current_character(info) == ']')) {
            del = current_character(info);
            move_to_left(info->contents[info->cy], info->cx, strlen(info->contents[info->cy]));
            info->cx--;
            del = current_character(info);
            move_to_left(info->contents[info->cy], info->cx, strlen(info->contents[info->cy]));
            update_cursor_pos(win, info);
        } else {
			info->cx--;
			del = current_character(info);
            move_to_left(info->contents[info->cy], info->cx, strlen(info->contents[info->cy]));
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
	unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
}

void enter_key_action(WINDOW *win, unblind_info_t *info, int add_to_ur_manager) {
	char c = '\n';
	if(strlen(current_line(info)) == 0) {
        array_insert(current_line(info), info->cx, c, info->size[info->cy]);
        if(strlen(info->contents[info->cy])+1 >= info->size[info->cy])  enlarge_characters_unblind_info(info, info->cy);
		info->cy++;
	} else {
        char *partition = (char *)malloc(info->size[info->cy] * sizeof(char));
        memset(partition, '\0', info->size[info->cy] * sizeof(char));

		int j = 0;
		for(int i = info->cx; i <= strlen(info->contents[info->cy]); i++) {
			partition[j] = info->contents[info->cy][i];
			j++;
		}
		int new_length = strlen(info->contents[info->cy]) - strlen(partition);
		info->contents[info->cy][new_length] = '\n';
		info->contents[info->cy][new_length+1] = '\0';
		
		int store = info->size[info->cy];
		
        info->size[info->cy] = MAX_CHARS_PER_LINE; // reset size
        while(info->size[info->cy] < new_length) info->size[info->cy] *= 2; // find apporiate size
        
        info->contents[info->cy] = (char *)realloc(info->contents[info->cy], info->size[info->cy] * sizeof(char));
		
        memset(info->contents[info->cy] + new_length + 2, '\0', info->size[info->cy] - new_length - 2);
		
        info->cy++;
        info->cx = 0;
		
		for(int k = MAX_LINES-1; k > info->cy; k--) {
            if(info->contents[k-1] == NULL) {
                memset(info->contents[k], '\0', info->size[k] * sizeof(char));
			} else {
                // this code is here because each line has it's own size so 
                // if you modify the size you need to realloc the size and this handles that
                char *par1 = (char *)malloc(info->size[k-1] * sizeof(char));
				
                // store line above
                strcpy(par1, info->contents[k-1]);
                
                
                // change line below to size of line above
                info->size[k] = info->size[k-1];
                
                info->contents[k] = realloc(info->contents[k], info->size[k] * sizeof(char));
                
                strncpy(info->contents[k], par1, info->size[k]);
                
                int len = strlen(info->contents[k]);
                memset(info->contents[k] + len+1, '\0', info->size[k] - len - 1);
                
                free(par1);
			}
		}
		info->size[info->cy] = store;
        info->contents[info->cy] = realloc(info->contents[info->cy], info->size[info->cy] * sizeof(char));
		strcpy(info->contents[info->cy], partition);
        int len = strlen(info->contents[info->cy]);
        memset(info->contents[info->cy] + len + 1, '\0', info->size[info->cy] - len - 1);
		free(partition);
	}
	
	if(info->contents[MAX_LINES-1][0] != '\0') enlarge_lines_unblind_info(info);
    
	unblind_scroll_vert_calc(win, info);
	unblind_scroll_hor_calc(win, info);
	
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
	if(c >= 32 && c <= 126) {
        if(!iscntrl(c)) { //TODO this if doesn't work
            switch(c) {
                case ')':
                    if(current_character(info) == ')') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
                case ']':
                    if(current_character(info) == ']') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
                case '>':
                    if(current_character(info) == '>') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
                case '\'':
        		case '\"':
                    if(current_character(info) == '\'' || current_character(info) == '\"') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
                case '}':
                    if(current_character(info) == '}') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
            }
            array_insert(info->contents[info->cy], info->cx, c, info->size[info->cy]);
            if(strlen(info->contents[info->cy])+1 >= info->size[info->cy])  enlarge_characters_unblind_info(info, info->cy);
			info->cx++;
			info->wcx++;
            unblind_scroll_hor_calc(win, info);
			// auto completion for ()'s and such
        	switch(c) {
        		case '(':
                    array_insert(info->contents[info->cy], info->cx, ')', info->size[info->cy]);
        			break;
        		case '[':
                    array_insert(info->contents[info->cy], info->cx, ']', info->size[info->cy]);
        			break;
        		case '{':
                    array_insert(info->contents[info->cy], info->cx, '}', info->size[info->cy]);
        			break;
        		case '\'':
        		case '\"':
                    array_insert(info->contents[info->cy], info->cx, c, info->size[info->cy]);
	       			break;
        		case '<':
                    array_insert(info->contents[info->cy], info->cx, '>', info->size[info->cy]);
        			break;
                
        	}
        	if(strlen(info->contents[info->cy])+1 >= info->size[info->cy])  enlarge_characters_unblind_info(info, info->cy);
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
	for(int i = 0; i < TAB_SIZE+1; i++) {
		array_insert(info->contents[info->cy], info->cx++, TAB_KEY, info->size[info->cy]);
        if(strlen(info->contents[info->cy])+1 >= info->size[info->cy])  enlarge_characters_unblind_info(info, info->cy);
	}
	if(add_to_ur_manager) {
		ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
		node->c = " "; // won't be used
		node->action = TAB;
		linked_list_d_add(info->ur_manager->stack_u, (void *) node, x, y);
	}
	unblind_scroll_hor_calc(win, info);
}

void duplicate_line(WINDOW *win, unblind_info_t *info, int add_to_ur_manager) {
    shift_down(win, info);
    info->size[info->cy-1] = info->size[info->cy];
    strcpy(info->contents[info->cy], info->contents[info->cy-1]);
    if(add_to_ur_manager == 1) {
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->c = " ";
        node->action = DUP_LINE;
        linked_list_d_add(info->ur_manager->stack_u, (void *) node, info->cx, info->cy);
    }
}

void delete_line(WINDOW *win, unblind_info_t *info, int add_to_ur_manager) {
    char *tmp = malloc(strlen(current_line(info)) * sizeof(char));
    tmp = strdup(current_line(info));
    int y = info->cy;
    int x = info->cx;
    int i = 0;
    while(info->contents[info->cy][i]) {
        move_to_left(info->contents[info->cy], i, strlen(info->contents[info->cy]));
    }
    if(info->cy == 0 && info->contents[info->cy+1][0] == '\0') {
        info->contents[info->cy][0] = '\n';
        info->contents[info->cy][1] = '\0';
        info->cx = 0;
    } else if(info->contents[info->cy + 1][0] == '\0') {
        info->cy--;
        info->wcy--;
    }
    shift_up(win, info);
    unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
    if(add_to_ur_manager == 1) {
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->c = malloc(strlen(tmp) * sizeof(char));
        node->c = strdup(tmp);
        free(tmp);
        node->action = DELETE_LINE;
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
    move_to_left(info->contents[y], x, strlen(info->contents[y]));
	info->cx = x;
    info->cy = y;
	unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
	linked_list_d_pop(info->ur_manager->stack_u);
}

void undo_backspace(WINDOW *win, unblind_info_t *info, char *c, int x, int y) {
	strcpy(info->message, c);
	info->cx = x;
	while(y != info->cy) {
		if(info->wcy < y) {
			info->cy++;
			info->wcy++;
	 	} else if(info->cy > y) {
			info->cy--;
			info->wcy--;
	 	}
	}
	unblind_scroll_vert_calc(win, info);
    unblind_scroll_hor_calc(win, info);
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
    unblind_scroll_hor_calc(win, info);
	linked_list_d_pop(info->ur_manager->stack_u);
}

void undo_enter(WINDOW *win, unblind_info_t *info, int y) {
	while(y != info->cy) {
		if(info->wcy < y) {
			info->cy++;
	 	} else if(info->cy > y) {
			info->cy--;
	 	}
	}
	unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
	info->cx = 0;
	info->wcx = 0;
	backspace_action(win, info, 0);
	linked_list_d_pop(info->ur_manager->stack_u);
}

void undo_tab(WINDOW *win, unblind_info_t *info, int x, int y) {
	int i = 0;
	while(i <= TAB_SIZE) {
        move_to_left(info->contents[y], x, strlen(info->contents[y]));
		info->cx = x;
		info->wcx = x;
		while(y != info->cy) {
			if(info->wcy < y) {
				info->cy++;
		 	} else if(info->cy > y) {
				info->cy--;
		 	}
		}
		unblind_scroll_hor_calc(win, info);
        unblind_scroll_vert_calc(win, info);
		i++;
	}
	linked_list_d_pop(info->ur_manager->stack_u);
}

void undo_delete_line(WINDOW *win, unblind_info_t *info, char *c, int x, int y) {
    info->cy = y-1;
    shift_down(win, info);
    info->cy = y;
    
    int new_length = strlen(c);
    info->size[info->cy] = MAX_CHARS_PER_LINE;
    while(info->size[info->cy] < new_length) info->size[info->cy] *= 2;

    strcpy(info->contents[info->cy], c);
    unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
    linked_list_d_pop(info->ur_manager->stack_u);
}
void undo_duplicate_line(WINDOW *win, unblind_info_t *info, int x, int y) {
    info->cx = x;
    info->cy = y;
    delete_line(win, info, 0);
    info->cy--;
    unblind_scroll_hor_calc(win, info);
    unblind_scroll_vert_calc(win, info);
    linked_list_d_pop(info->ur_manager->stack_u);
}
