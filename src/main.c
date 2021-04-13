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
#include "actions.h"
#include "mainframe.h"
#include "messages.h"

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

void create_win(unblind_info_t *info) {
    if(windows == 0) {
        win[windows] = newwin(LINES, COLS, 0, 0);
        info->win = win[windows];
        info->winlines = LINES;
        info->wincols = COLS;
        noecho();
        nodelay(win[windows], FALSE);
        keypad(win[windows], FALSE);
        scrollok(win[windows], FALSE);
        raw();
        windows++;
    } else {
        for(int i = 0; i < windows-1; i++) {
            wresize(win[i], LINES/windows, COLS/windows);
            info->winlines /= windows;
            info->wincols /= windows;
        }
        win[windows] = newwin(LINES, COLS, 0, 0);
        info->win = win[windows];
        noecho();
        nodelay(win[windows], FALSE);
        keypad(win[windows], FALSE);
        scrollok(win[windows], FALSE);
        raw();
        windows++;
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
    create_win(info);

    if(argc == 2) {
        file_name = argv[1];
        f = fopen(file_name, "r");
    } else {
        endwin();
        fprintf(stderr, USAGE);
        exit(1);
    }
    if(f != NULL) {
        read_contents_from_file(f, win[0], info);
    }
    if(strlen(current_line(info)) == 0) {
        current_line(info)[0] = '\n';
    }
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
