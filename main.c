#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>


#include "unblind.h"

WINDOW *win;
FILE *f;
char *file_name;

int main(int argc, char *argv[]) {
	WINDOW *holder = initscr();
    win = newwin(MAX_LINES, 99, 0, 0);
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	//immedok(win, TRUE);
	scrollok(win, FALSE);
	//idlok(win, FALSE);
	//nl();
	//wsetscrreg(win, MAX_LINES, 0);
    if(argc == 2) {
        file_name = argv[1];
        f = fopen(file_name, "a+"); // opens for reading and appending
        if(f == NULL) {
            fprintf(stderr, "Error: File not found!");
            endwin();
            exit(1);
        }
    } else {
        fprintf(stderr, "Error: ./unblind <file name>");
        endwin();
        exit(1);
    }
    read_contents_from_file(f, win);
	draw(win);
    for(;;) {
        manage_input(file_name, win);
        wrefresh(win);
    }
	
    endwin();
	return 0;
}