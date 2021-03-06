#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include "unblind.h"
#include "mainframe.h"

int main(int argc, char *argv[]) {
    print_to_log("Program start");

	MAX_LINES = 4096;
	MAX_CHARS_PER_LINE = 256;
	INFO_SIZE = MAX_LINES * MAX_CHARS_PER_LINE * 2; // TODO this can probably be a lot smaller
	WINDOW_HEIGHT = LINES_PER_WINDOW*2;

    FILE *f;
    char *file_name;
    WINDOW *win;
    //WINDOW **windows; // stores all windows
    //int max_windows = 8;
    //*windows = malloc(sizeof(WINDOW *) * 8); // this should allow 8 windows

    unblind_info_t *info = (unblind_info_t *) malloc(4096); // TODO modified this should use INFO_SIZE
    setup_unblind_info(info);

    print_to_log("info created");

    initscr();
    win = newwin(MAX_LINES, MAX_CHARS_PER_LINE, 0, 0);
    noecho();
    nodelay(stdscr, FALSE);
    keypad(stdscr, FALSE);
    scrollok(win, FALSE);
    raw();

    if(argc == 2) {
        file_name = argv[1];
        f = fopen(file_name, "a+"); // opens for reading and appending
        if(f == NULL) {
            fprintf(stderr, "Error: File not found!");
            endwin();
            exit(1);
        }
    } else {
        endwin();
        fprintf(stderr, "Error: ./unblind <file name>\n");
        exit(1);
    }

    read_contents_from_file(f, win, info);
    print_to_log("right before draw\n");
    draw(win, info);
    print_to_log("Starting main loop\n");
    struct timespec fps;
    fps.tv_sec = 0;
    fps.tv_nsec = 33333333.333; // about 30 fps
    for(;;) {
        nanosleep(&fps, NULL);
//         print_to_log("Main loop start\n");
        manage_input(file_name, win, info);
        draw(win, info);
    }
    fclose(f);
    endwin();
    exit(0);
    return 0;
}
