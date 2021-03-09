#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "double_linked_list.h"
#include "unblind.h"
#include "actions.h"
#include "mainframe.h"

void unblind_scroll_vert_calc(WINDOW *win, unblind_info_t *info) {
    if(info->cy > SCROLL_THRESHOLD) {
        info->scroll_offset = info->cy - SCROLL_THRESHOLD;
    } else {
        info->scroll_offset = 0;
    }
    info->wcy = info->cy-info->scroll_offset;
    draw(win, info);
}

void unblind_scroll_hor_calc(WINDOW *win, unblind_info_t *info, int natural) {
    if(info->cx > SCROLLX_THRESHOLD) {
        info->scrollX_offset = info->cx - SCROLLX_THRESHOLD;
    } else {
        info->scrollX_offset = 0;
    }
    info->wcx = info->cx-info->scrollX_offset;
    
    draw(win, info);
}


void draw(WINDOW *win, unblind_info_t *info) {
	if(!info->contents) return;
	werase(win); // this was causing errors with the windows ubuntu system
	int y;
	for(int i = info->scroll_offset; i <= info->scroll_offset + LINES; i++) {
		y = i - info->scroll_offset;
		int x = 0;
		for(int j = info->scrollX_offset; j <= info->scrollX_offset + COLS; j++) {
            x = j-info->scrollX_offset;
            if(info->contents[i][0] == '\0') break;
			if(info->contents[i][j] != '\0') {
				// bottom two lines are used for messages and other things
				if(y <= LINES-PROTECTED_LINES) {
				    wmove(win, y, x);
					waddch(win, info->contents[i][j]);
				}
			} else {
                break;
            }
		}
	}
	wmove(win, LINES-2, 0);
	mvprintw(LINES-2, 0, info->message);
	wrefresh(win);
	update_cursor_pos(win, info);
}

void read_contents_from_file(FILE *f, WINDOW *win, unblind_info_t *info) {
    int i = 0; // characters
    int j = 0; // lines
	int amount_to_read = MAX_CHARS_PER_LINE;
	char *str = malloc(sizeof(char) * amount_to_read);
	
	while(fgets(str, amount_to_read, f) != NULL) {
		if(j+2 >= MAX_LINES) {
        	enlarge_lines_unblind_info(info);
        }
        
        int sizeStr = strlen(str) + 1;
        if(str[strlen(str) - 1] == '\n') {
 			strcat(info->contents[j], str);
			i = 0;
			j++;
		} else {
			i += sizeStr;
			strcat(info->contents[j], str);
		}
		if(strlen(info->contents[j]) + sizeStr + 1 >= info->size[j])  enlarge_characters_unblind_info(info, j);
		
	}
	
    fclose(f);
    free(str);
    for(int k = 0; k < MAX_LINES-1; k++) {
        for(int l = 0; l < info->size[k]-1; l++) {
            if(info->contents[k][l] == '\0') break;
            if(info->contents[k][l] == TAB_KEY) {
                for(int o = 0; o < TAB_SIZE; o++) {
                    array_insert(info->contents[k], l, TAB_KEY, info->size[k]);
                    if(strlen(info->contents[k])+1 >= info->size[k])  enlarge_characters_unblind_info(info, k);
					l++;
				}
            }
        }
    }
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
			if(c == TAB_KEY) {
				j += TAB_SIZE;
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
    mvprintw(LINES-1, COLS - 18, "pos: %4d, %4d ", info->wcx + info->scrollX_offset, info->wcy + info->scroll_offset);
    mvprintw(LINES-1, COLS - 31, "char: ----");
    if(current_character(info) == '\n') {
    	mvprintw(LINES-1, COLS - 31, "char: \\n");
    } else {
    	mvprintw(LINES-1, COLS - 31, "char: %c", info->contents[info->cy][info->cx]);	
    }
    
    move(info->wcy, info->wcx);
	refresh();
	print_to_log("done updating cursor pos...");
}

void manage_input(char *file_name, WINDOW *win, unblind_info_t *info) {
	print_to_log("managing input...\n");
	//reset_unblind_info_contents(info);
	char c = getch();
	if(info->m == FIND) {
		if(c == ENTER_KEY) {
			find_str(win, info);
			info->m = EDIT;
            // memset(info->message, '\0', MAX_JUMP_STR_LENGTH * sizeof(char));
            update_cursor_pos(win, info);
		} else if(c == BACKSPACE_KEY_0 || c == BACKSPACE_KEY_1 || c == BACKSPACE_KEY_2) {
            if(strlen(info->fstr) == 0) return;
			info->fstr[strlen(info->fstr)-1] = '\0';
            info->wcx--;
			strcpy(info->message, info->fstr);
			draw(win, info);
		} else if((c >= 'A' && c <= 'z')) {
			if(strlen(info->fstr)+1 == sizeof(char) * FIND_STR_MAX_LENGTH) return; // this is very long shouldn't need to be any bigger
			info->fstr[strlen(info->fstr)] = c;
            info->wcx++;
			strcpy(info->message, info->fstr);
			draw(win, info);
		}
		return;
	} else if(info->m == JUMP) {
        //char *line = malloc(1000 * sizeof(char)); // if a line number is bigger than this I don't know what to tell ya
        if(c == ENTER_KEY) {
            int lineNum = atoi(info->jstr); //TODO put -1 if I ever change the position to show the first line as 1, atm it's 0 so this works better
            if(lineNum < 0) {
                info->m = EDIT;
                return;
            }
            jump_to_line(win, info, lineNum);
            memset(info->message, '\0', MAX_JUMP_STR_LENGTH * sizeof(char));
            info->m = EDIT;
            update_cursor_pos(win, info);
        } else if(c == BACKSPACE_KEY_0 || c == BACKSPACE_KEY_1 || c == BACKSPACE_KEY_2) {
            if(strlen(info->jstr) == 0) return;
            info->jstr[strlen(info->jstr)-1] = '\0';
            info->wcx--;
            strcpy(info->message, info->jstr);
            draw(win, info);
        } else if((c >= '0' && c <= '9')) {
            if(strlen(info->jstr)+1 == 1000 * sizeof(char)) return;
            info->jstr[strlen(info->jstr)] = c;
            info->wcx++;
            strcpy(info->message, info->jstr);
            draw(win, info);
        }
        return;
    }
	if(c == ERR) {
		return; // DON'T REDRAW SCREEN
	}
	
	int x = c;

	
	/* 
	* This code looks messy it just handles ctrl arrow keys and special keys like page up and page down
	* since ncurses doesn't have a way to check for ctrl arrow keys this has to be done in order to detect them.
	* this makes detecting other keys more difficult, also some systems send different key combos for the same keys
	* so that makes it worse
	*/
	if(c == 27) {
		c = getch();
		if(c == 91) { // is not a ctrl arrow key
			c = getch();
			if(c == 53 || c == 54) {
				x = PAGE_MOD + c;
				c = getch();
			} else {	
				if(c == 49) {
					getch();
					getch();
					c = getch();
					x = CTRL_ARROW_KEY_MOD + c;
				} else {
					x = ARROW_KEY_MOD + c;
				}
			}
		} else if(c == 79) { // is a ctrl arrow key
			c = getch();
			x = CTRL_ARROW_KEY_MOD + c;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	
	switch(x) {
		case PAGE_UP:
			strcpy(info->message, "page up!");
            //draw(win, info);
			break;
		case PAGE_DOWN:
			strcpy(info->message, "page down!");
            //draw(win, info);
			break;
		case CTRL_DOWN_ARROW:
            jump_to_end(win, info);
			break;
		case CTRL_UP_ARROW:
            jump_to_start(win, info);
			break;
		case CTRL_RIGHT_ARROW:
            if(current_line(info)[strlen(current_line(info))-1] == '\n') info->cx = strlen(current_line(info))-1;
            else info->cx = strlen(current_line(info));
			unblind_scroll_hor_calc(win, info, 0);
			break;
		case CTRL_LEFT_ARROW:
			info->cx = 0;
			unblind_scroll_hor_calc(win, info, 0);
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
			info->find = NULL;
			memset(info->fstr, '\0', sizeof(char) * FIND_STR_MAX_LENGTH);
			memset(info->message, '\0', MAX_MESSAGE_LENGTH * sizeof(char));
            unblind_move_to_message(win, info);
            //draw(win, info);
			break;
        case CTRL_B:
            memset(info->jstr, '\0', MAX_JUMP_STR_LENGTH * sizeof(char));
            memset(info->message, '\0', MAX_JUMP_STR_LENGTH * sizeof(char));
            info->m = JUMP;
            unblind_move_to_message(win, info);
            //draw(win, info);
            break;
		case CTRL_P:
			find_str(win, info);
			//next_find_str(win, info);
            //draw(win, info);
			break;
		case CTRL_Q: // ctrl-q
			shutdown(win, info);
			break;
		case CTRL_S: // ctrl-s
			save_file(file_name, info);
			break;
		case BACKSPACE_KEY_0:
		case BACKSPACE_KEY_1:
		case BACKSPACE_KEY_2:
			backspace_action(win, info, 1);
            //draw(win, info);
			break;
		case TAB_KEY:
			tab_action(win, info, 1);
            //draw(win, info);
			break;
		case ENTER_KEY:
			enter_key_action(win, info, 1);
            //draw(win, info);
			break;
		case CTRL_D: // ctrl-d
			duplicate_line(win, info);
            //draw(win, info);
			break;
		case CTRL_X: // ctrl-x
			print_to_log("deleting line...\n");
			delete_line(win, info);
			print_to_log("Shifting up...\n");
			shift_up(win, info);
            //draw(win, info);
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
			//draw(win, info);
			break;
		default:
			type_char(win, c, info, 1);
            //draw(win, info);
			break;
	}
// 	draw(win, info);
	
}

void duplicate_line(WINDOW *win, unblind_info_t *info) {
	shift_down(win, info);
	strcpy(info->contents[info->cy], info->contents[info->cy-1]);
}

void delete_line(WINDOW *win, unblind_info_t *info) {
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
	unblind_scroll_hor_calc(win, info, 0);
}

void move_to_left(char *arr, int left, int size) {
    for(int j = left; j < size; j++) {
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
		char *par1 = (char *)malloc(info->size[i] * sizeof(char));
        strcpy(par1, info->contents[i]);
        
        info->contents[i] = realloc(info->contents[i], info->size[i] * sizeof(char));
        
        info->size[i-1] = info->size[i];
        info->contents[i-1] = realloc(info->contents[i-1], info->size[i-1] * sizeof(char));
        
		strcpy(info->contents[i-1], par1);
		//strcpy(info->contents[i], "");
	}
    if(current_line(info)[strlen(current_line(info))-1] == '\n') info->cx = strlen(current_line(info))-1;
    else info->cx = strlen(current_line(info));
    
    unblind_scroll_vert_calc(win, info);
    unblind_scroll_hor_calc(win, info, 0);
}

void shift_down(WINDOW *win, unblind_info_t *info) {
	for(int i = MAX_LINES-1; i > info->cy; i--) {
		strcpy(info->contents[i], info->contents[i-1]);
	}
	info->cy++;
	info->wcy++;
    unblind_scroll_vert_calc(win, info);
}

int array_insert(char *a, int x, char c, int size)
{
    if(a == NULL) return 0;
    for(int i = strlen(a)+1; i >= 0; i--) {
        if(i == x) {
            a[i] = c;
            return 1;
        }
        a[i] = a[i-1];
    }
    return 0;
}

void unblind_move_to_message(WINDOW *win, unblind_info_t *info) {
    info->wcy = LINES-2;
    info->wcx = 0;
    update_cursor_pos(win, info);
}

void print_to_log(const char *error) {
	//TODO remove this file later, kind of annoying
    FILE *fedit = fopen(".unblind_log.txt", "w+");
        if(fedit == NULL) {
            fprintf(stderr, "Error: log file not found\n");
            return;
        }
		if(strlen(error)) {
			fputs(error, fedit);
		} else if(error[0] == '\n') {
			fputs("\n", fedit);
		}
    fclose(fedit);
}
