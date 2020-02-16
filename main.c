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
    win = initscr();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
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
    for(;;) {
        manage_input(file_name, win);
    }
	
    endwin();
	return 0;
}