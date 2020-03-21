#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "unblind.h"
#include "actions.h"

/*
void *find_word(char *str, char **contents) {
	for(int i = 0; i < MAX_LINES; i++) {
		for(int k = 0; k < strlen(contents[i])-strlen(str); i++) {
			for(int j = 0; j < strlen(str); j++) {
				//TODO		
			}
		}
		
	}
	return NULL;
}
*/

void unblind_scroll_down(WINDOW *win, unblind_info_t *info) {
	if(!(info->cy+1 >= MAX_LINES)) {
		info->scroll_offset++;
		update_cursor_pos(win, info);
		draw(win, info);
	}
}

void unblind_scroll_up(WINDOW *win, unblind_info_t *info) {
	if(!(info->cy-1 <= -1)) {
		info->scroll_offset--;
		update_cursor_pos(win, info);
		draw(win, info);
	}
}

void draw(WINDOW *win, unblind_info_t *info) {
	if(!info->contents) return;
	werase(win);
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
            	info->contents[j][i++] = 9;
            	info->contents[j][i++] = 9;
            	info->contents[j][i++] = 9;
            	info->contents[j][i] = 9;
            	i++;
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
       
		/*if(strlen(c)) {
			fputs(c, fedit);
		} else if(c[0] == '\n') {
			fputs("\n", fedit);
		}*/
    }
    fclose(fedit);
}

void update_cursor_pos(WINDOW *win, unblind_info_t *info) {
    mvprintw(LINES-1, COLS - 13, "pos: %d, %d ", info->cx, info->cy);
    mvprintw(LINES-1, COLS - 26, "char: ----");
    mvprintw(LINES-1, COLS - 26, "char: %c", info->contents[info->cy][info->cx]);
	//wcy = cy%LINES_PER_WINDOW;
    move(info->wcy, info->cx);
	refresh();
	//wrefresh(win);
}

void manage_input(char *file_name, WINDOW *win, unblind_info_t *info) {
	char c = getch();
    //TODO maybe make this a switch
	if(c == EOF) {
		return; // DON'T REDRAW SCREEN
	}
	switch(c) {
		case 2: // down arrow
			move_cursor_down(win, info);
			break;
		case 3: // up arrow
			move_cursor_up(win, info);
			break;
		case 4: // left arrow
			move_cursor_left(win, info);
			break;
		case 5: // right arrow
			move_cursor_right(win, info);
			break;
		case 17: // ctrl-q
			shutdown(info);
			break;
		case 19: // ctrl-s
			save_file(file_name, info);
			break;
		case 127:
		case 8:
		case 7:
			backspace_action(win, info);
			break;
		case 9:
			tab_action(win, info);
			break;
		case 10:
			enter_key_action(win, info);
			break;
		default:
			type_char(c, info);
			break;
	}
	
	draw(win, info);
}

void delete_line(WINDOW *win, int y, unblind_info_t *info) {
	int i = 0;
	while(info->contents[y][i]) {
		wmove(win, y, i);
		wdelch(win);
		move_to_left(info->contents[y], i);
	}
}

void move_to_left(char *arr, int left) {
    for(int j = left; j < MAX_CHARS_PER_LINE; j++) {
        arr[j] = arr[j + 1];
    }
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
	if(a[x]) return 1;
	return 0;
}

void print_to_log(const char *error) {
    FILE *fedit = fopen("log.txt", "w+");
       // FILE *fedit2 = fopen(file_name, "a");
		if(strlen(error)) {
			fputs(error, fedit);
		} else if(error[0] == '\n') {
			fputs("\n", fedit);
		}
        //fclose(fedit);
    fclose(fedit);
}

void setup_unblind_info(unblind_info_t *info) {
	info->cx = 0;
	info->cy = 0;
	info->contents = (char **)malloc(MAX_LINES * sizeof(char *));
	for(int i = 0; i < MAX_LINES; i++) {
		info->contents[i] = (char *)malloc(MAX_CHARS_PER_LINE * sizeof(char));
		memset(info->contents[i], 0, MAX_CHARS_PER_LINE * sizeof(char));
	}
	info->scroll_offset = 0;
	info->wcy = 0;
	info->wcx = 0;
	info->message = (char *)malloc(MAX_CHARS_PER_LINE * sizeof(char));
}

void unblind_info_free(unblind_info_t *info) {
	for(int i = 0; i < MAX_LINES; i++) {
		free(info->contents[i]);
	}
	free(info->contents);
	free(info->message);
	free(info);
}

void shutdown(unblind_info_t *info) {
	endwin();
	unblind_info_free(info);
	exit(0);
}
