#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "double_linked_list.h"
#include "unblind.h"
#include "actions.h"
#include "mainframe.h"
#include "messages.h"
#include "csv_parse.h"

void unblind_scroll_vert_calc(unblind_info_t *info) {
    if(info->cy > info->y_threshold) {
        info->scroll_offset = info->cy - info->y_threshold;
    } else {
        info->scroll_offset = 0;
    }
    info->wcy = info->cy-info->scroll_offset;
}

void unblind_scroll_hor_calc(unblind_info_t *info) {
    if(info->cx > info->x_threshold) {
        info->scrollX_offset = info->cx - info->x_threshold;
    } else {
        info->scrollX_offset = 0;
    }
    info->wcx = info->cx-info->scrollX_offset;
}


void draw(unblind_info_t *info) {
	if(!info->contents) return;
	//werase(info->win); // this was causing errors with the windows ubuntu system
	
	color_t color = 0;
	int toggleColor = 0;
	int y;
	int x;
	int j;
	int i;
    for(i = info->scroll_offset; i < info->scroll_offset + info->winlines; i++) {
		y = i - info->scroll_offset;
		x = 0;
			for(j = info->scrollX_offset; j < info->scrollX_offset + info->wincols-1; j++) {
				x = j - info->scrollX_offset;
				if(info->size[i] < j) break; // no need to draw
				if(info->contents[i][0] == '\0') break;
				
				if(toggleColor == 0) {
					for(int k = 0; k < info->p_data->wordCount; k++) {
						int len = strlen(info->p_data->words[k]);
						char *tmp = malloc(len+1 * sizeof(char));
						tmp[len+1] = '\0';
						if(j > 0 && (info->contents[i][j] != '"')) {
							// checks to make sure the word is stand alone
							if(info->contents[i][j-1] != ' ' &&
								info->contents[i][j-1] != '\t' &&
								info->contents[i][j-1] != '(' &&
								info->contents[i][j-1] != ')') continue;
						}
						int infront = j+len;
						if(infront <= strlen(info->contents[i]) && (info->contents[i][j] != '"')) {
							if(info->contents[i][infront] != ' ' &&
								info->contents[i][infront] != '\t' &&
								info->contents[i][infront] != '(' &&
								info->contents[i][infront] != ')' &&
								info->contents[i][infront] != '\n' &&
								info->contents[i][infront] != '\0') {
								continue;
							}
						}
						strncpy(tmp, info->contents[i]+j, len);
						if(strncmp(tmp, info->p_data->words[k], len) == 0) {
							color = info->p_data->colors[k];
							toggleColor = len;
						}
					}
				}
				
				if(info->contents[i][j] != '\0') {
					// bottom two lines are used for messages and other things
					if(y <= info->winlines - PROTECTED_LINES) {
						switch(color) {
							case RED:
								wattron(info->win, COLOR_PAIR(3));
								break;
							case MAGENTA:
								wattron(info->win, COLOR_PAIR(2));
								break;
							case GREEN:
								wattron(info->win, COLOR_PAIR(4));
								break;
							case BLUE:
								wattron(info->win, COLOR_PAIR(5));
								break;
						}
						wmove(info->win, y, x);
						waddch(info->win, info->contents[i][j]);
						switch(color) {
							case RED:
								wattroff(info->win, COLOR_PAIR(3));
								break;
							case MAGENTA:
								wattroff(info->win, COLOR_PAIR(2));
								break;
							case GREEN:
								wattroff(info->win, COLOR_PAIR(4));
								break;
							case BLUE:
								wattroff(info->win, COLOR_PAIR(5));
								break;
						}
					}
					if(toggleColor > 0) toggleColor--;
					if(toggleColor == 0) color = 0;
				} else {
					break;
				}
			}
	}
	x = info->wincols-1;
	for(int m = 0; m < y; m++) {
		wmove(info->win, m, x);
		waddch(info->win, '|');
	}
	wmove(info->win, info->winlines - 2, 0);
	//wrefresh(info->win);
    mvwprintw(info->win, info->winlines - 2, 0, info->message);
	update_cursor_pos(info);
    wmove(info->win, info->wcy, info->wcx);
	wrefresh(info->win);
}

void read_contents_from_file(FILE *f, unblind_info_t *info) {
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
    draw(info);
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

void update_cursor_pos(unblind_info_t *info) {
    mvwprintw(info->win, info->winlines - 1, info->wincols - 31, "pos: %4d, %4d ", info->cx, info->cy);
    mvwprintw(info->win, info->winlines - 1, info->wincols - 15, "char: ----");
    if(current_character(info) == '\n') {
        mvwprintw(info->win, info->winlines - 1, info->wincols - 15, "char: \\n");
    } else {
        mvwprintw(info->win, info->winlines - 1, info->wincols - 15, "char: %c", info->contents[info->cy][info->cx]);
    }
    
	wrefresh(info->win);
}

void manage_input(char *file_name, unblind_info_t *info, char c, th_info_t *th) {
	if(info->m == FIND) {
		if(c == ENTER_KEY) {
			find_str(info);
			info->m = EDIT;
            // memset(info->message, '\0', MAX_JUMP_STR_LENGTH * sizeof(char));
            update_cursor_pos(info);
		} else if(c == BACKSPACE_KEY_0 || c == BACKSPACE_KEY_1 || c == BACKSPACE_KEY_2) {
            if(strlen(info->fstr) == 0) return;
			info->fstr[strlen(info->fstr)-1] = '\0';
            info->wcx--;
			strcpy(info->message, info->fstr);
		} else if((c >= 32 && c <= 126)) {
			if(strlen(info->fstr)+1 == sizeof(char) * FIND_STR_MAX_LENGTH) return; // this is very long shouldn't need to be any bigger
			info->fstr[strlen(info->fstr)] = c;
            info->wcx++;
			strcpy(info->message, info->fstr);
		} else if(c == ESC_KEY) {
			info->m = EDIT;
			unblind_scroll_vert_calc(info);
			unblind_scroll_hor_calc(info);
			update_cursor_pos(info);
			memset(info->fstr, '\0', sizeof(char) * FIND_STR_MAX_LENGTH);
			memset(info->message, '\0', MAX_MESSAGE_LENGTH * sizeof(char));
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
            jump_to_line(info, lineNum);
            memset(info->message, '\0', MAX_JUMP_STR_LENGTH * sizeof(char));
            info->m = EDIT;
            update_cursor_pos(info);
        } else if(c == BACKSPACE_KEY_0 || c == BACKSPACE_KEY_1 || c == BACKSPACE_KEY_2) {
            if(strlen(info->jstr) == 0) return;
            info->jstr[strlen(info->jstr)-1] = '\0';
            info->wcx--;
            strcpy(info->message, info->jstr);
        } else if((c >= '0' && c <= '9')) {
            if(strlen(info->jstr)+1 == 1000 * sizeof(char)) return;
            info->jstr[strlen(info->jstr)] = c;
            info->wcx++;
            strcpy(info->message, info->jstr);
		} else if(c == ESC_KEY) {
			info->m = EDIT;
			unblind_scroll_vert_calc(info);
			unblind_scroll_hor_calc(info);
			update_cursor_pos(info);
			memset(info->message, '\0', MAX_MESSAGE_LENGTH * sizeof(char));
		}
        return;
    } else if(info->m == CMD) {
        if(c == ENTER_KEY) {
            info->m = EDIT;
            unblind_scroll_vert_calc(info);
            unblind_scroll_hor_calc(info);
            update_cursor_pos(info);
            
            th->infos[th->windows] = (unblind_info_t *) malloc(sizeof(unblind_info_t));
            setup_unblind_info(th->infos[th->windows]);
            
            strcpy(th->infos[th->windows]->file_name, info->cmd);
            
            if(create_win(th) == 1) {
                set_active_window(th, th->windows-1);
            } else {
                unblind_info_free_mini(th->infos[th->windows]);
                strcpy(info->message, NER);
            }
        } else if(c == BACKSPACE_KEY_0 || c == BACKSPACE_KEY_1 || c == BACKSPACE_KEY_2) {
            if(strlen(info->cmd) == 0) return;
            info->cmd[strlen(info->cmd)-1] = '\0';
            strcpy(info->message, info->cmd);
            info->wcx--;
        } else if((c >= 32 && c <= 126)) {
            if(strlen(info->cmd)+1 == sizeof(char) * FIND_STR_MAX_LENGTH) return; // this is very long shouldn't need to be any bigger
            info->cmd[strlen(info->cmd)] = c;
            info->wcx++;
            strcpy(info->message, info->cmd);
		} else if(c == ESC_KEY) {
			info->m = EDIT;
			unblind_scroll_vert_calc(info);
			unblind_scroll_hor_calc(info);
			update_cursor_pos(info);
			memset(info->message, '\0', MAX_MESSAGE_LENGTH * sizeof(char));
		}
        return;
	} else if(info->m == QUIT_SAVE) {
		// move cursor to end of current string
		info->wcx = strlen(info->message);
		info->prompt_save = 0;
		if(c == 'y' && info->message[info->wcx-1] != 'y' && info->message[info->wcx-1] != 'n') {
			info->message[info->wcx++] = 'y';
			info->prompt_save = 1;
		} else if(c == 'n' && info->message[info->wcx-1] != 'y' && info->message[info->wcx-1] != 'n') {
			info->message[info->wcx++] = 'n';
			info->prompt_save = 0;
		} else if((c == BACKSPACE_KEY_0 || c == BACKSPACE_KEY_1 || c == BACKSPACE_KEY_2) &&
			(info->message[info->wcx-1] == 'y' || info->message[info->wcx-1] == 'n')) {
			info->wcx--; 
			info->message[info->wcx] = '\0';
		} else if(c == ESC_KEY) {
			info->m = EDIT;
			unblind_scroll_vert_calc(info);
			unblind_scroll_hor_calc(info);
			update_cursor_pos(info);
		} else if(c == ENTER_KEY) {
			if(info->prompt_save == 1) {
				save_file(file_name, info);
			}
			if(th->windows == 1) shutdown(th);
			else close_active_win(th);
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
	int modified = 0; // was the file modified during operation?
	switch(x) {
		case PAGE_UP:
            jump_to_start(info);
			break;
		case PAGE_DOWN:
            jump_to_end(info);
			break;
		case CTRL_DOWN_ARROW:
            move_line_down(info, 1);
			modified = 1;
			break;
		case CTRL_UP_ARROW:
            move_line_up(info, 1);
			modified = 1;
			break;
		case CTRL_RIGHT_ARROW:
            if(current_line(info)[strlen(current_line(info))-1] == '\n') info->cx = strlen(current_line(info))-1;
            else info->cx = strlen(current_line(info));
			unblind_scroll_hor_calc(info);
			break;
		case CTRL_LEFT_ARROW:
			info->cx = 0;
			unblind_scroll_hor_calc(info);
			break;
		case DOWN_ARROW: // down arrow
			move_cursor_down(info);
			break;
		case UP_ARROW: // up arrow
			move_cursor_up(info);
			break;
		case LEFT_ARROW: // left arrow
			move_cursor_left(info);
			break;
		case RIGHT_ARROW: // right arrow
			move_cursor_right(info);
			break;
        case CTRL_E:
            //TODO prompt for a file name and a direction (vert or hor)
            memset(info->cmd, '\0', sizeof(char) * FIND_STR_MAX_LENGTH);
            memset(info->message, '\0', MAX_MESSAGE_LENGTH * sizeof(char));
            info->m = CMD;
            unblind_move_to_message(info);
            break;
		case CTRL_F:
			info->m = FIND;
			info->find = NULL;
			memset(info->fstr, '\0', sizeof(char) * FIND_STR_MAX_LENGTH);
			memset(info->message, '\0', MAX_MESSAGE_LENGTH * sizeof(char));
            unblind_move_to_message(info);
			break;
        case CTRL_B:
            memset(info->jstr, '\0', MAX_JUMP_STR_LENGTH * sizeof(char));
            memset(info->message, '\0', MAX_JUMP_STR_LENGTH * sizeof(char));
            info->m = JUMP;
            unblind_move_to_message(info);
            break;
		case CTRL_P:
			find_str(info);
			break;
		case CTRL_Q: // ctrl-q
			if(info->needs_saved ==  1) {
				info->m = QUIT_SAVE;
				unblind_move_to_message(info);
				strcpy(info->message, SAVE_QUIT_MESG);
				info->wcx = strlen(info->message);
			} else {
				if(th->windows == 1) shutdown(th);
				else close_active_win(th);
			}
			break;
		case CTRL_S: // ctrl-s
			save_file(file_name, info);
			info->needs_saved = 0;
			break;
		case BACKSPACE_KEY_0:
		case BACKSPACE_KEY_1:
		case BACKSPACE_KEY_2:
			backspace_action(info, 1);
			modified = 1;
			break;
		case TAB_KEY:
			tab_action(info, 1);
			modified = 1;
			break;
		case ENTER_KEY:
			enter_key_action(info, 1);
			modified = 1;
			break;
		case CTRL_D: // ctrl-d
			duplicate_line(info, 1);
			modified = 1;
			break;
        case CTRL_W:
            change_active_window(th);
            break;
		case CTRL_X: // ctrl-x
			delete_line(info, 1);
			modified = 1;
			break;
		case CTRL_Z: // ctrl-z
			if(info->ur_manager->stack_u->tail != NULL) {
				dll_node_t *node = (dll_node_t *) info->ur_manager->stack_u->tail;
				ur_node_t *ur_node = (ur_node_t *) node->value;
                switch(ur_node->action) {
                    case TYPE:
                        undo_type_char(info, node->x, node->y);
                        break;
                    case BACKSPACE:
                        undo_backspace(info, ur_node->c, node->x, node->y);
                        break;
                    case BACKSPACE_LAST_CHAR:
                        undo_last_backspace(info, ur_node->c, node->x, node->y);
                        break;
                    case TAB:
                        undo_tab(info, node->x, node->y);
                        break;
                    case ENTER:
                        undo_enter(info, ur_node->c, node->y);
                        break;
                    case DELETE_LINE:
                        undo_delete_line(info, ur_node->c, node->x, node->y);
                        break;
                    case DUP_LINE:
                        undo_duplicate_line(info, node->x, node->y);
                        break;
                    case MOVE_LINE_DOWN:
                        undo_move_line_down(info, node->x, node->y);
                        break;
                    case MOVE_LINE_UP:
                        undo_move_line_up(info, node->x, node->y);
                        break;
                    default:
                        break;
                }
			}
			modified = 1;
			break;
		default:
			type_char(c, info, 1);
			modified = 1;
			break;
	}
	if(modified == 1) {
		info->needs_saved = 1;
	}
}

void move_to_left(char *arr, int left, int size) {
    for(int j = left; j < size; j++) {
        arr[j] = arr[j + 1];
    }
}

void shift_up(unblind_info_t *info) {
	if(info->cy == 0 && info->contents[info->cy+1][0] == '\0') { // at the top of the file, return
		return;
	}
	for(int i = info->cy+1; i < MAX_LINES; i++) {
		if(info->contents[i][0] == '\0') {
			break;
		}
		char *par1 = (char *)malloc(info->size[i] * sizeof(char));
        strcpy(par1, info->contents[i]);
        
        memset(info->contents[i], 0 ,info->size[i] * sizeof(char));
        
        info->size[i-1] = info->size[i];
        info->contents[i-1] = realloc(info->contents[i-1], info->size[i-1] * sizeof(char));
        
		strcpy(info->contents[i-1], par1);
        free(par1);
	}
    if(current_line(info)[strlen(current_line(info))-1] == '\n') info->cx = strlen(current_line(info))-1;
    else info->cx = strlen(current_line(info));
    
    unblind_scroll_vert_calc(info);
    unblind_scroll_hor_calc(info);
}

void shift_down(unblind_info_t *info) {
	for(int i = MAX_LINES-1; i > info->cy; i--) {
        char *par1 = (char *)malloc(info->size[i-1] * sizeof(char));
        strcpy(par1, info->contents[i-1]);
        
        info->size[i] = info->size[i-1];
        info->contents[i] = realloc(info->contents[i], info->size[i] * sizeof(char));
        memset(info->contents[i], 0,info->size[i] * sizeof(char));
		strcpy(info->contents[i], par1);
        free(par1);
	}
	info->cy++;
	info->wcy++;
    unblind_scroll_vert_calc(info);
}

language_t get_file_type(unblind_info_t *info) {
	int size = 1024;
	char *buf = calloc(size, sizeof(char));
	strcpy(buf, info->file_name);
	strtok(buf, ".");
	buf = strtok(NULL, ".");
	if(buf == NULL) return UNKNOWN;
	if(strcmp(buf, "c") == 0 || strcmp(buf, "h") == 0) { //TODO add other language support
		return C;
	} else if(strcmp(buf, "js") == 0) {
		return JS;
	} else if(strcmp(buf, "py") == 0) {
		return PYTHON;
	} else if(strcmp(buf, "java") == 0) {
		return JAVA;
	}
	return UNKNOWN;
}

color_t get_color(char *color) {
	if(strcmp(color, "red") == 0) {
		return RED;
	} else if(strcmp(color, "blue") == 0) {
		return BLUE;
	} else if(strcmp(color, "green") == 0) {
		return GREEN;
	}  else if(strcmp(color, "magenta") == 0) {
		return MAGENTA;
	}
	return 0;
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

void unblind_move_to_message(unblind_info_t *info) {
    info->wcy = info->winlines-2;
    info->wcx = 0;
    wmove(info->win, info->wcy, info->wcx);
    wrefresh(info->win);
}

void set_active_window(th_info_t *th, int value) {
    if(th->activeWin + 1 == th->windows) th->activeWin = 0;
    else th->activeWin = value;
}

void change_active_window(th_info_t *th) {
    if(th->activeWin + 1 >= th->windows) {
        th->activeWin = 0;
    } else {
        th->activeWin++;
    }
}

void draw_all_screens(th_info_t *th) {
    for(int i = 0; i < th->windows; i++) {
        draw(th->infos[i]);
    }
}

