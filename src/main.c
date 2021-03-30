#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "unblind.h"
#include "mainframe.h"

pthread_mutex_t lock;
char *file_name;
int windows;
WINDOW **win;
unblind_info_t *info;

void inputThread() {
    for(;;) {
        char c = getch();
        pthread_mutex_lock(&lock);
        for(int i = 0; i < windows; i++) {
            manage_input(file_name, win[i], info, c);
        }
        pthread_mutex_unlock(&lock);
    }
}

void drawThread() {
    struct timespec fps;
    fps.tv_sec = 0;
    fps.tv_nsec = 33333333.333; // about 30 fps
    for(;;) {
        nanosleep(&fps, NULL);
        pthread_mutex_lock(&lock);
        for(int i = 0; i < windows; i++) {
            draw(win[i], info);
        }
        pthread_mutex_unlock(&lock);
    }
}

void create_win() {
    if(windows == 0) {
        win[windows++] = newwin(LINES, COLS, 0, 0);
        noecho();
        nodelay(win[windows-1], FALSE);
        keypad(win[windows-1], FALSE);
        scrollok(win[windows-1], FALSE);
        raw();
    } else {
        for(int i = 0; i < windows-1; i++) {
            wresize(win[i], LINES/windows, COLS/windows);
        }
        win[windows++] = newwin(LINES, COLS, 0, 0);
        noecho();
        nodelay(win[windows-1], FALSE);
        keypad(win[windows-1], FALSE);
        scrollok(win[windows-1], FALSE);
        raw();
        
    }
    
}

int main(int argc, char *argv[]) {
	MAX_LINES = 4096;
	MAX_CHARS_PER_LINE = 256;
	INFO_SIZE = MAX_LINES * MAX_CHARS_PER_LINE * 2; // TODO this can probably be a lot smaller
	WINDOW_HEIGHT = LINES_PER_WINDOW*2;
    windows = 0;
    
    win = malloc(sizeof(WINDOW *) * 8);
    
    FILE *f;

    info = (unblind_info_t *) malloc(sizeof(unblind_info_t));
    setup_unblind_info(info);

    initscr();
    create_win();

    if(argc == 2) {
        file_name = argv[1];
        f = fopen(file_name, "a+");
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

    read_contents_from_file(f, win[0], info);
    draw(win[0], info);
    
    int tid[THREADS];
    pthread_t th[THREADS];
    pthread_mutex_init(&lock, NULL);
    pthread_create(&th[0], NULL, (void *) inputThread, &tid[0]);
    pthread_create(&th[1], NULL, (void *) drawThread, &tid[1]);
    for(int i = 0; i < THREADS; i++) {
        pthread_join(th[i], NULL);
    }
    fclose(f);
    endwin();
    exit(0);
    return 0;
}
