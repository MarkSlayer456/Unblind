#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "unblind.h"

#define LINES_PER_WINDOW 36

int cx = 0; // cursor x
int cy = 0; // cursor y
int wcy = 0; // relative to screen pos

int scroll_offset = 0;

char contents[MAX_LINES][MAX_CHARS_PER_LINE]; // this is a lot of memory

char message[255] = "";

typedef struct point {
	int y;
	int x;
} point_t;

point_t *find_word(char *str) {
	for(int i = 0; i < MAX_LINES; i++) {
		for(int k = 0; k < strlen(contents[i])-strlen(str); i++) {
			for(int j = 0; j < strlen(str); j++) {
				//TODO		
			}
		}
		
	}
	return NULL;
}

void unblind_scroll_down(WINDOW *win, int y) {
	if(!(y+1 >= MAX_LINES)) {
		scroll_offset++;
		//cy--;
		update_cursor_pos(win);
		draw(win);
	}
}

void unblind_scroll_up(WINDOW *win, int y) {
	if(!(y-1 <= -1)) {		
		scroll_offset--;
		//cy++;
		update_cursor_pos(win);
		draw(win);
	}
}

void draw(WINDOW *win) {
	werase(win);
	int y;
	for(int i = scroll_offset; i < MAX_LINES; i++) {
		y = i - scroll_offset;
		for(int j = 0; j <= strlen(contents[i]); j++) {
			if(contents[i][j] != '\0') {
				//mvdelch(y, (j+1)); not sure why this was here
				// bottom two lines are used for messages and other things
				if(y <= LINES-2) {
				    wmove(win, y, j);
					waddch(win, contents[i][j]);
				}
				//TODO
				//wprintmv(win, message);
			}			
		}			
	}
	wmove(win, 35, 0);
	mvprintw(35, 0, message);
	wrefresh(win);
	update_cursor_pos(win);
	//wrefresh(win);
}

void read_contents_from_file(FILE *f, WINDOW *win) {
    char c;
    int i = 0;
    int j = 0;
    while((c = getc(f)) != EOF) {
        if(c == '\n') {
            // add char
            contents[j][i] = c;
			contents[j][i+1] = '\0';
            j++;
            i = 0;
        } else if(c == '\0') {
			if(i == 0) {
				contents[j][i] = '\n';
				contents[j][i+1] = '\0';
				j++;
				i = 0;	
			} else {
				contents[j][i] = '\0';
				j++;
				i = 0;				
			}
		} else if(c >= 32 && c <= 126) {
            // add char
            contents[j][i] = c;
            i++;
        } else {
			contents[j][i] = c;
			contents[j][i+1] = '\0';
            j++;
            i = 0;
		}
		//FILE *f1 = fopen("log.txt", "a+");
			//fputc(c, f1);
			//fclose(f1);
    }
    update_cursor_pos(win);
    fclose(f);
}

void write_contents_to_file(char *file_name) {
    FILE *fedit = fopen(file_name, "w+");
    for(int i = 0; i < MAX_LINES; i++) {
       // FILE *fedit2 = fopen(file_name, "a");
        char *c = contents[i];
		if(strlen(c)) {
			fputs(c, fedit);
		} else if(c[0] == '\n') {
			fputs("\n", fedit);
		}
        //fclose(fedit);
    }
    fclose(fedit);
}

void update_cursor_pos(WINDOW *win) {
    int y;
    int x;
    getmaxyx(stdscr, y, x);
    mvprintw(y - 1, x - 13, "pos: %d, %d ", cx, cy);
	//wcy = cy%LINES_PER_WINDOW;
    move(wcy, cx);
	refresh();
}

void move_cursor(enum directions d, WINDOW *win) {
	switch(d) {
        case LEFT:
			if(contents[cy][cx-1] && cx-1 != -1) {
				cx--;
			} else if(cx-1 == -1 && !(cy-1 <= -1)) {
				cx = strlen(contents[--cy])-1;	
				wcy--;
			}
			if(cy == 0 && cx == 0) {
				unblind_scroll_up(win, wcy);
			}
            break;
        case RIGHT:
			if(contents[cy][cx] != '\n' && contents[cy][cx] != '\0') {
				cx++;
			} else if((contents[cy+1][0] || contents[cy+1][0] == '\n') && !(cy+1 >= MAX_LINES)) {
				cx = 0;
				cy++;
				wcy++;
			}
			if(LINES_PER_WINDOW-1 == cy-scroll_offset && cx == strlen(contents[cy])) {
					unblind_scroll_down(win, wcy);
			}
            break;
        case DOWN:
			if(cx <= strlen(contents[cy+1]) && contents[cy+1][0] != '\0') {
				cy++;
				wcy++;
			} else if(contents[cy+1][0] != '\0') {
				cy++;
				wcy++;
				cx = strlen(contents[cy])-1;
			}
			if(LINES-6 <= wcy && contents[wcy-1][0] != '\0') {
				//cy--;
				wcy--;
				unblind_scroll_down(win, wcy);
			}
            break;
        case UP:
			if(!(cy-1 <= -1)) {
				cy--;
				wcy--;
				if(cx > strlen(contents[cy])-1) {
					cx = strlen(contents[cy])-1;
				} else if(contents[cy][0] == '\n') {
					cx = 0;
				}
				if(wcy <= 6 && scroll_offset > 0) {
					wcy++;
					unblind_scroll_up(win, wcy);
				}
			}
            break;
    }
	update_cursor_pos(win);
}

void manage_input(char *file_name, WINDOW *win) {
    char c = getch();
    //int new_cy = cy+scroll_offset;
    //TODO maybe make this a switch
    enum directions d;
    if(c == 5) { // right
        d = RIGHT;
        move_cursor(d, win);
    } else if(c == 4) { // left
        d = LEFT;
        move_cursor(d, win);
    } else if(c == 3) { // up
        d = UP;
        move_cursor(d, win);
    } else if(c == 2) { // down
        d = DOWN;
        move_cursor(d, win);
    } else if(c == 19) { //ctrl-s
        write_contents_to_file(file_name);
        strcat(message, "Saved ");
        strcat(message, file_name);
		//TODO save the file
	} else if(c == 17) { // ctrl-q 
		shutdown();
	} else if(c == 127 || c == 8 || c == 7) { // backspace or delete and bell?
        if(cx <= 0 && cy <= 0) return;
		if(cx <= 0 && cy > 0) {
			if(cy == 0) {
				unblind_scroll_up(win, cy);
			}
			//cy--;
		    int x = strlen(contents[--cy]);
            cx = x;
			mvwdelch(win, cy, --cx);
			contents[cy][cx] = '\0';
			strcat(contents[cy], contents[cy+1]);
			//draw_contents(contents[cy], cy);
			delete_line(win, cy + 1);
			strcpy(contents[cy+1], "\n");
			//new_cy++;
			cy++;
			//strcpy(contents[cy], "\n");
			for(int y = cy; y < MAX_LINES-1; y++) {
				strcpy(contents[y], contents[y+1]);
			}
			
			//new_cy--;
			if(wcy <= 6 && scroll_offset > 0) {
				wcy++;
				unblind_scroll_up(win, wcy);
			}
			cy--;
			wcy--;
		} else {
            move_to_left(contents[cy], --cx);
            update_cursor_pos(win);
		}
		
	} else if(c == 10) { //TODO enter key
			c = '\n';
			if(strlen(contents[cy]) == 0) {
				if(!str_insert(contents[cy], cx, c)) {
					contents[cy][cx] = c;
					cy++;
					//new_cy++;
				}
			} else {
				char partition[MAX_CHARS_PER_LINE] = "";
				int j = 0;
				for(int i = cx; i <= strlen(contents[cy]); i++) {
					partition[j] = contents[cy][i];
					j++;
				}
				int new_length = strlen(contents[cy]) - strlen(partition);
				contents[cy][new_length] = '\n';
				contents[cy][new_length+1] = '\0';
				//new_cy++;
				cy++;
				cx = 0;
				for(int y = MAX_LINES; y > cy; y--) {
					strcpy(contents[(y)], contents[y-1]);
				}
				strcpy(contents[cy], partition);
			}
			if(LINES-6 <= wcy && contents[wcy-1][0] != '\0') {
				wcy--;
				unblind_scroll_down(win, wcy);
			}
			wcy++;
			//cy--;
    } else if(c != EOF) {
        if(!iscntrl(c)) {
			array_insert(contents[cy], cx, c);
			cx++;
			// auto completion for ()'s and such
        	switch(c) {
        		case '(':
        			array_insert(contents[cy], cx, ')');
        			break;
        		case '[':
        			array_insert(contents[cy], cx, ']');
        			break;
        		case '{':
        			array_insert(contents[cy], cx, '}');
        			break;
        		case '\'':
        		case '\"':
        			array_insert(contents[cy], cx, c);
	       			break;
        		case '<':
        			array_insert(contents[cy], cx, '>');
        			break;
        	}
        }
        strcpy(message, "");
	}
	
    update_cursor_pos(win);
	draw(win);
}

void delete_line(WINDOW *win, int y) {
	int i = 0;
	while(contents[y][i]) {
		wmove(win, y, i);
		wdelch(win);
		//mvwdelch(win, y, i);
		move_to_left(contents[y], i);
	}
	update_cursor_pos(win);
}


char *array_merge(char *arr1, char *arr2) {
    int i = strlen(arr1)+1; // the new line character matters here
    int j = 0;
    while(arr2[j] != '\0') {
		int index = i+j;
        arr1[index] = arr2[j];
        j++;
    }
	return arr1;
	refresh();
}

void move_to_left(char *arr, int left) {
    for(int j = left; j < MAX_CHARS_PER_LINE; j++) {
        arr[j] = arr[j + 1];
    }
}

void array_insert(char *a, int x, char c) {
	char par[255] = "";
	int j = 0;
	for(int i = x; i < strlen(a); i++) {
		par[j] = a[i];
		j++;
	}
	a[x] = c;
	a[x+1] = '\0';
	strcat(a, par);
}

// clears a spot in the array for insertion
int str_insert(char *arr, int insert, char c) {
    int i = 0;
    while(arr[i] != '\0') {
        if(i == insert) {
            for(int j = MAX_CHARS_PER_LINE; j > i; j--) {
                arr[j] = arr[j - 1];
            }
            arr[i] = c;
            break;
        }
        i++;
    }
    if(arr[i]) return 1;
    return 0;
}

void shutdown() {
	endwin();
	exit(0);
}
