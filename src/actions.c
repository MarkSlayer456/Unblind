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
#include "messages.h"

void jump_to_start(unblind_info_t *info) {
    info->wcy = 0;
    info->cy = 0;
    info->cx = 0;
    info->wcx = 0;
    unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
    update_cursor_pos(info);
}

void jump_to_end(unblind_info_t *info) {
    int i = 0;
    
	for(; i < info->max_lines; i++) {
        if(info->contents[i][0] == '\0') break;
    }
    i--;
    info->cy = i;
    if(current_line(info)[strlen(current_line(info))-1] == '\n') info->cx = strlen(current_line(info))-1;
    else info->cx = strlen(current_line(info));
    unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
    
    update_cursor_pos(info);
}

void jump_to_line(unblind_info_t *info, int line) {
    if(info->contents[line][0] == '\0') {
        strcpy(info->message, LINE_DOES_NOT_EXIST);
    } else {
        info->cy = line;
        info->cx = 0;
    }
	unblind_scroll_hor_calc(info);
	unblind_scroll_vert_calc(info);
}

void jump_forward_word(unblind_info_t *info) {
	int prev_space = 0;
	int curr_char = 0;
	for(;;) {
		if(current_character(info) == ' ' || current_character(info) == '\n' || current_character(info) == '\t') prev_space = 1;
		if(current_character(info) != ' ' && current_character(info) != '\n' && current_character(info) != '\t' && prev_space) curr_char = 1;
		if(prev_space && curr_char) break;
		move_cursor_right(info);
	}
}

void jump_backward_word(unblind_info_t *info) {
	int curr_space = 0;
	int prev_char = 0;
	for(;;) {
		move_cursor_left(info);
		if(current_character(info) != ' ' && current_character(info) != '\n' && current_character(info) != '\t') prev_char = 1;
		if((current_character(info) == ' ' || current_character(info) == '\n' || current_character(info) == '\t') && prev_char) curr_space = 1;
		if(curr_space && prev_char) {
			move_cursor_right(info);
			break;
		}
		if(info->cx == 0 && info->cy == 0) break;
	}
}

void move_cursor_up(unblind_info_t *info) {
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
    unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
}

void move_cursor_down(unblind_info_t *info) {
	if(next_line(info)[info->cx] == TAB_KEY) {
		info->cy++;
		info->wcy++;
		while(current_character(info) == '\0') {
			move_cursor_left(info);
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
	unblind_scroll_hor_calc(info);
	unblind_scroll_vert_calc(info);
}

void move_cursor_left(unblind_info_t *info) {
	if(info->cx-1 == -1 && !(info->cy-1 <= -1)) {
		--info->cy;
		info->cx = strlen(current_line(info))-1;
		info->wcy--;
	} else if(info->cx-1 == -1) {
		return;
	} else if(info->contents[info->cy][info->cx-1] == TAB_KEY) {
        info->cx--;
        if(info->cx == 0) {
            move_cursor_up(info);
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
	}
	unblind_scroll_vert_calc(info);
	unblind_scroll_hor_calc(info);
}

void move_cursor_right(unblind_info_t *info) {
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
	unblind_scroll_vert_calc(info);
	unblind_scroll_hor_calc(info);
}

int hash(char *str) {
    int hash_prime = 301;
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
void find_str(unblind_info_t *info) {
	dll_node_t *tmp = NULL;
	if(info->find == NULL) {
		info->find = linked_list_d_create();

		int find = hash(info->fstr);
		int Fsize = strlen(info->fstr);
		for(int j = 0; j < info->max_lines; j++) {
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
			strcpy(info->message, NO_RESULTS);
            unblind_scroll_hor_calc(info);
            unblind_scroll_vert_calc(info);
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
        unblind_scroll_hor_calc(info);
        unblind_scroll_vert_calc(info);
    }
}

void backspace_action(unblind_info_t *info, int add_to_ur_manager) {
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
        
        info->size[info->cy] = DEFAULT_MAX_CHARS_PER_LINE;
        while(info->size[info->cy] < new_length) info->size[info->cy] *= 2;
        info->contents[info->cy] = realloc(info->contents[info->cy], info->size[info->cy] * sizeof(char));
		
        strcat(info->contents[info->cy], info->contents[info->cy+1]);
        info->cy++;
        int fl = 1;
        if(next_line(info)[0] == '\0') { // this is to counter act the movement from delete line
            fl = 0;
        }
		delete_line(info, 0);
        if(fl) info->cy--;
        info->cx = len-1;
		if(add_to_ur_manager) {
			ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
			char *del1 = &del;
			node->c = (char *) malloc(sizeof(char)); // won't be used
			memset(node->c, '\0', sizeof(char));
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
            (prev_character(info) == '[' && current_character(info) == ']') ||
            (prev_character(info) == '<' && current_character(info) == '>') ||
            (prev_character(info) == '\'' && current_character(info) == '\'')) {
            del = current_character(info);
            move_to_left(info->contents[info->cy], info->cx, strlen(info->contents[info->cy]));
            info->cx--;
            del = current_character(info);
            move_to_left(info->contents[info->cy], info->cx, strlen(info->contents[info->cy]));
        } else {
			info->cx--;
			del = current_character(info);
            move_to_left(info->contents[info->cy], info->cx, strlen(info->contents[info->cy]));
		}
		if(add_to_ur_manager) {
			ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
			char *del1 = &del;
			node->c = (char *) malloc(sizeof(char)); // won't be used
			memset(node->c, '\0', sizeof(char));
			node->c = strdup(del1);
			node->action = BACKSPACE;
			linked_list_d_add(info->ur_manager->stack_u, (void *) node, info->cx, info->cy);
		}
	}
	unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
}

void enter_key_action(unblind_info_t *info, int add_to_ur_manager) {
	char c = '\n';
	int tabs = 0;
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
		
		info->size[info->cy] = DEFAULT_MAX_CHARS_PER_LINE; // reset size
        while(info->size[info->cy] < new_length) info->size[info->cy] *= 2; // find apporiate size
        
        info->contents[info->cy] = (char *)realloc(info->contents[info->cy], info->size[info->cy] * sizeof(char));
		
        memset(info->contents[info->cy] + new_length + 2, '\0', info->size[info->cy] - new_length - 2);
		
        // detmine how many tabs before
        for(int i = 0; i < strlen(current_line(info)); i++) {
            if(info->contents[info->cy][i] == '\t') {
                tabs += 1;
                i += TAB_SIZE;
            }
            if(info->contents[info->cy][i] != '\t') break;
        }
        info->cy++;
        info->cx = 0;
		
        
        
		for(int k = info->max_lines-1; k > info->cy; k--) {
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
        for(int i = 0; i < tabs; i++) {
            tab_action(info, 0);
        }
	}
	
	if(info->contents[info->max_lines-1][0] != '\0') enlarge_lines_unblind_info(info);
    
	unblind_scroll_vert_calc(info);
	unblind_scroll_hor_calc(info);
	
    if(add_to_ur_manager == 1) {
		ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->c = malloc(512 * sizeof(char));
        for(int i = 0; i < tabs; i++) {
            node->c[i] = '\t'; 
        }
		node->action = ENTER;
		linked_list_d_add(info->ur_manager->stack_u, (void *) node, info->cx,info->cy);
	}
}

void save_file(char *file_name, unblind_info_t *info) {
	write_contents_to_file(file_name, info);
	strcpy(info->message, EMPTY);
	strcat(info->message, SAVED);
	strcat(info->message, file_name);
}

void type_char(char c, unblind_info_t *info, int add_to_ur_manager) {
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
                    if(current_character(info) == '\'') {
                        info->cx++;
                        info->wcx++;
                        return;
                    }
                    break;
        		case '\"':
                    if(current_character(info) == '\"') {
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
            unblind_scroll_hor_calc(info);
			// auto completion for ()'s and such
			if(add_to_ur_manager == 1) {
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
					case '"':
						array_insert(info->contents[info->cy], info->cx, c, info->size[info->cy]);
						break;
				}
			}
        	if(strlen(info->contents[info->cy])+1 >= info->size[info->cy])  enlarge_characters_unblind_info(info, info->cy);
        }
        strcpy(info->message, "");
		if(add_to_ur_manager == 1) {
			ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
			node->c = strdup(&c); // need so redos can happen
			node->action = TYPE;
			linked_list_d_add(info->ur_manager->stack_u, (void *) node, x, y);
		}
	} else {
		return;
	}
}

void tab_action(unblind_info_t *info, int add_to_ur_manager) {
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
	unblind_scroll_hor_calc(info);
}

void duplicate_line(unblind_info_t *info, int add_to_ur_manager) {
    shift_down(info);
    info->size[info->cy-1] = info->size[info->cy];
    strcpy(info->contents[info->cy], info->contents[info->cy-1]);
    if(add_to_ur_manager == 1) {
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->c = " ";
        node->action = DUP_LINE;
        linked_list_d_add(info->ur_manager->stack_u, (void *) node, info->cx, info->cy);
    }
}

void delete_line(unblind_info_t *info, int add_to_ur_manager) {
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
    shift_up(info);
    unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
    if(add_to_ur_manager == 1) {
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->c = malloc(strlen(tmp) * sizeof(char));
        node->c = strdup(tmp);
        free(tmp);
        node->action = DELETE_LINE;
        linked_list_d_add(info->ur_manager->stack_u, (void *) node, x, y);
    }
}

void move_line_down(unblind_info_t *info, int add_to_ur_manager) {
    if(info->contents[info->cy+1][0] == '\0') {
        return;
    }
    char *tmp = calloc(info->size[info->cy], sizeof(char));
    char *tmp2 = calloc(info->size[info->cy+1], sizeof(char));
    strcpy(tmp, info->contents[info->cy]);
	strcpy(tmp2, info->contents[info->cy+1]);
    long long int tmpSize = info->size[info->cy];
    
    info->size[info->cy] = info->size[info->cy+1];
    info->size[info->cy+1] = tmpSize;
    
    info->contents[info->cy] = realloc(info->contents[info->cy], info->size[info->cy]);
    info->contents[info->cy+1] = realloc(info->contents[info->cy+1], info->size[info->cy+1]);
    
    
    strcpy(info->contents[info->cy], tmp2);
    strcpy(info->contents[info->cy+1], tmp);
	memset(info->contents[info->cy]+strlen(tmp2), '\0', info->size[info->cy] - strlen(tmp2));
	memset(info->contents[info->cy+1]+strlen(tmp), '\0', info->size[info->cy+1] - strlen(tmp));
	
	free(tmp);
	free(tmp2);
	
    info->cy++;
    unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
	
    if(add_to_ur_manager == 1) {
		int x = info->cx;
		int y = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = MOVE_LINE_DOWN;
        linked_list_d_add(info->ur_manager->stack_u, (void *) node, x, y);
    }
}

void move_line_up(unblind_info_t *info, int add_to_ur_manager) { //TODO change move_line_down to this format looks cleaner
    if(info->cy == 0) { // can't move up from here
        return;
    }
    char *tmp = calloc(info->size[info->cy], sizeof(char));
	char *tmp2 = calloc(info->size[info->cy-1], sizeof(char));
    strcpy(tmp, info->contents[info->cy]);
	strcpy(tmp2, info->contents[info->cy-1]);
	
	long long int tmpSize = info->size[info->cy];
	
	info->size[info->cy] = info->size[info->cy+1];
	info->size[info->cy-1] = tmpSize;
	
	info->contents[info->cy] = realloc(info->contents[info->cy], info->size[info->cy]);
	info->contents[info->cy-1] = realloc(info->contents[info->cy-1], info->size[info->cy-1]);
	
    strcpy(info->contents[info->cy], tmp2);
    strcpy(info->contents[info->cy-1], tmp);
	memset(info->contents[info->cy]+strlen(tmp2), '\0', info->size[info->cy] - strlen(tmp2));
	memset(info->contents[info->cy-1]+strlen(tmp), '\0', info->size[info->cy-1] - strlen(tmp));
    free(tmp);
	free(tmp2);
    info->cy--;
    unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
	
    if(add_to_ur_manager == 1) {
		int x = info->cx;
		int y = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = MOVE_LINE_UP;
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

void undo_type_char(unblind_info_t *info, int x, int y) {
    move_to_left(info->contents[y], x, strlen(info->contents[y]));
	info->cx = x;
    info->cy = y;
	unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
		int rx = info->cx;
		int ry = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
	ur_node_t *undo_node = info->ur_manager->stack_u->tail->value;
        node->action = TYPE;
	node->c = strdup(undo_node->c);
	linked_list_d_add(info->ur_manager->stack_r, (void *) node, rx, ry);
	linked_list_d_pop(info->ur_manager->stack_u);
}

void undo_backspace(unblind_info_t *info, char *c, int x, int y) {
	strcpy(info->message, UNDO);
	info->cx = x;
    info->cy = y;
	unblind_scroll_vert_calc(info);
    unblind_scroll_hor_calc(info);
	if(*c == TAB_KEY) {
		tab_action(info, 0);
	} else {
		type_char(*c, info, 0);
	}
	linked_list_d_pop(info->ur_manager->stack_u);
		int rx = info->cx;
		int ry = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = BACKSPACE;
	linked_list_d_add(info->ur_manager->stack_r, (void *) node, rx, ry);
}

void undo_last_backspace(unblind_info_t *info, char *c, int x, int y) {
	strcpy(info->message, c);
	info->cx = x;
	info->cy = y;
	enter_key_action(info, 0);
    unblind_scroll_hor_calc(info);
	linked_list_d_pop(info->ur_manager->stack_u);
		int rx = info->cx;
		int ry = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = BACKSPACE_LAST_CHAR;
	linked_list_d_add(info->ur_manager->stack_r, (void *) node, rx, ry);
}

void undo_enter(unblind_info_t *info, char *c, int y) {
    info->cy = y;
    info->cx = strlen(c) * (TAB_SIZE+1); // because tab_size is broken apparently
    backspace_action(info, 0);
    for(int i = 0; i < strlen(c); i++) {
        backspace_action(info, 0);
    }
    unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
	linked_list_d_pop(info->ur_manager->stack_u);
		int rx = info->cx;
		int ry = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = ENTER;
	linked_list_d_add(info->ur_manager->stack_r, (void *) node, rx, ry);
}

void undo_tab(unblind_info_t *info, int x, int y) {
	int i = 0;
	while(i <= TAB_SIZE) {
        move_to_left(info->contents[y], x, strlen(info->contents[y]));
		info->cx = x;
        info->cy = y ;
		unblind_scroll_hor_calc(info);
        unblind_scroll_vert_calc(info);
		i++;
	}
	linked_list_d_pop(info->ur_manager->stack_u);
		int rx = info->cx;
		int ry = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = TAB;
	linked_list_d_add(info->ur_manager->stack_r, (void *) node, rx, ry);
}

void undo_delete_line(unblind_info_t *info, char *c, int x, int y) {
	if(y == 0) {
		shift_down(info);
		info->cy = y;
	} else {
		info->cy = y-1;
		shift_down(info);
		info->cy = y;
	}
    int new_length = strlen(c);
    info->size[info->cy] = DEFAULT_MAX_CHARS_PER_LINE;
    while(info->size[info->cy] < new_length) info->size[info->cy] *= 2;

    strcpy(info->contents[info->cy], c);
    info->cx = strlen(c)-1;
    unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
    linked_list_d_pop(info->ur_manager->stack_u);
		int rx = info->cx;
		int ry = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = DELETE_LINE;
	linked_list_d_add(info->ur_manager->stack_r, (void *) node, rx, ry);

}

void undo_duplicate_line(unblind_info_t *info, int x, int y) {
    info->cx = x;
    info->cy = y;
    delete_line(info, 0);
    info->cy--;
    unblind_scroll_hor_calc(info);
    unblind_scroll_vert_calc(info);
    linked_list_d_pop(info->ur_manager->stack_u);
		int rx = info->cx;
		int ry = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = DUP_LINE;
	linked_list_d_add(info->ur_manager->stack_r, (void *) node, rx, ry);
}

void undo_move_line_down(unblind_info_t *info, int x, int y) {
    info->cx = x;
    info->cy = y;
    move_line_up(info, 0);
    linked_list_d_pop(info->ur_manager->stack_u);
		int rx = info->cx;
		int ry = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = MOVE_LINE_DOWN;
	linked_list_d_add(info->ur_manager->stack_r, (void *) node, rx, ry);
}

void undo_move_line_up(unblind_info_t *info, int x, int y) {
    info->cx = x;
    info->cy = y;
    move_line_down(info, 0);
    linked_list_d_pop(info->ur_manager->stack_u);
		int rx = info->cx;
		int ry = info->cy;
        ur_node_t *node = (ur_node_t *)malloc(sizeof(ur_node_t));
        node->action = MOVE_LINE_UP;
	linked_list_d_add(info->ur_manager->stack_r, (void *) node, rx, ry);
}
