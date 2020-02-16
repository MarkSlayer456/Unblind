#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "unblind.h"

int cx = 0; // cursor x
int cy = 0; // cursor y

char contents[MAX_LINES][MAX_CHARS_PER_LINE]; // this is a lot of memory

void read_contents_from_file(FILE *f, WINDOW *win) {
    char c;
    int i = 0;
    int j = 0;
    while((c = getc(f)) != EOF) {
        mvaddch(cy, cx, c);
        getyx(win, cy, cx);
        if(c == '\n') {
            // add char
            contents[j][i] = c;
            j++;
            i = 0;
        } else {
            // add char
            contents[j][i] = c;
            i++;
        }

    }
    update_cursor_pos(win);
    fclose(f);
}

void write_contents_to_file(char *file_name) {
    FILE *fedit = fopen(file_name, "w+");
    // clears the file
    fclose(fedit);

    for(int i = 0; i < MAX_LINES; i++) {
        FILE *fedit2 = fopen(file_name, "a");
        char *c = contents[i];
        fputs(c, fedit2);
        fclose(fedit);
    }
}

void update_cursor_pos(WINDOW *win) {
    int y;
    int x;
    getmaxyx(stdscr, y, x);
    mvprintw(y - 1, x - 13, "pos: %d, %d ", cx, cy);
    wmove(win, cy, cx);
    getyx(win, cy, cx);
}

int length(char *arr) {
    int i = 0;
    char c = arr[0];
    while(c) {
        if(c != '\n') {
            i++;
            c = arr[i];
        } else {
			i++;
			break;
		}
    }

    return i;
}

void move_cursor(enum directions d, WINDOW *win) { //TODO make sure character isn't null when moving (require there to be at least a space)
    switch(d) {
        case LEFT:
            wmove(win, cy, --cx);
            break;
        case RIGHT:
            wmove(win, cy, ++cx);
            break;
        case DOWN:
            wmove(win, ++cy, cx);
            break;
        case UP:
            wmove(win, --cy, cx);
            break;
    }
    getyx(win, cy, cx);
}

void draw_contents(char *cont, int cy) {
	int y = cy;
	for(int i = 0; i < MAX_CHARS_PER_LINE; i++) {
		if(cont[i]) {
			mvaddch(cy, i, cont[i]);
		} 
		//else {
			//mvaddch(cy, i, '\0');
		//}
	}
}

void array_merge(char *arr1, char *arr2) {
    int i = length(arr1);
    int j = 0;
	//TODO this is where is error is
    while(arr2[j]) {
		int index = i+j;
        arr1[index] = arr2[j];
        j++;
    }
	mvprintw(0, 50, "1");
	refresh();
}

void move_to_left(char *arr, int left) {
    for(int j = left; j < MAX_CHARS_PER_LINE; j++) {
        arr[j] = arr[j + 1];
    }
}

// clears a spot in the array for insertion
int array_insert(char *arr, int insert, char c) {
    int i = 0;
    while(arr[i]) {
        if(i == insert) {
            for(int j = MAX_CHARS_PER_LINE; j > i; j--) {
                // if(arr[j - 1] == 0) continue;
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

void manage_input(char *file_name, WINDOW *win) {
    char c = getch();
    // if(c != -1) {
    //     mvprintw(0, 25, "character value: %d", c);
    //     wmove(win, cy, cx);
    // }
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
    } else if(c == 23) { //ctrl-w
        write_contents_to_file(file_name);
		//TODO save the file
	} else if(c == 127 || c == 8 || c == 7) { // backspace or delete and bell?
        if(cx <= 0 && cy <= 0) return;
		if(cx <= 0 && cy > 0) {
            int x = length(contents[--cy]);
            cx = x;
			array_merge(contents[cy], contents[cy+1]);
			draw_contents(contents[cy], cy);
			delete_line(win, cy + 1);
		} else {
            mvwdelch(win, cy, --cx);
            move_to_left(contents[cy], cx);
            update_cursor_pos(win);
		}
	} else if(c == 10) { //TODO enter key
            c = '\n';
			// insert into array
			// move line to the right to the next line
			
			//if(array_insert(contents[cy], cx, c)) {
				//cy++;
				//update_cursor_pos(win);
				//for(int i = 0; i < MAX_CHARS_PER_LINE; i++) {
					//if(contents[cy][i] == 0) continue;
					//mvaddch(cy, i, contents[cy][i]);
				//}
				//cx++;
				//update_cursor_pos(win);
				//wmove(win, cy, cx++);
            //}
			
            mvaddch(cy, cx, c);
            //contents[cy++][cx] = c;
            cy++;
			cx = 0;
            update_cursor_pos(win);
    } else if(c != EOF) {
        if(!iscntrl(c)) {
                if(array_insert(contents[cy], cx, c)) {
                    for(int i = cx; i < MAX_CHARS_PER_LINE; i++) {
                        if(contents[cy][i] == 0) continue;
                        mvaddch(cy, i, contents[cy][i]);
                    }
                    wmove(win, cy, cx++);
                } else {
                    mvaddch(cy, cx, c);
                    contents[cy][cx++] = c;
                }
            }
	}
    update_cursor_pos(win);
}

void delete_line(WINDOW *win, int y) {
	int i = 0;
	while(contents[y][i]) {
		mvwdelch(win, y, i);
		move_to_left(contents[y], i);
	}
	update_cursor_pos(win);
}
