#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <math.h>

#include "unblind.h"
#include "actions.h"
#include "mainframe.h"
#include "messages.h"

int main(int argc, char *argv[]) {
    int windows = 0;
    int activeWin = 0;
    unblind_info_t **infos = (unblind_info_t **) malloc(sizeof(unblind_info_t *) * MAX_WINDOWS);
    int drawAll = 0;
    pthread_mutex_t lock;
    
    infos[windows] = (unblind_info_t *) malloc(sizeof(unblind_info_t));
    setup_unblind_info(infos[windows]);

    
    pthread_t th[THREADS];
    pthread_mutex_init(&lock, NULL);
    
    th_info_t *th_info = malloc(sizeof(th_info_t));
    th_info->windows = windows;
    th_info->activeWin = activeWin;
    th_info->drawAll = drawAll;
    th_info->infos = infos;
    th_info->lock = &lock;
    
    if(argc == 2) {
        th_info->infos[th_info->activeWin]->file_name = strdup(argv[1]);
    } else {
        endwin();
        fprintf(stderr, USAGE);
        exit(1);
    }
    
    initscr();
	create_win(th_info);
    
	start_color();
	use_default_colors();
	
	init_pair(1, -1, -1);
	init_pair(2, COLOR_MAGENTA, -1);
	init_pair(3, COLOR_RED, -1);
	init_pair(4, COLOR_GREEN, -1);
	init_pair(5, COLOR_BLUE, -1);
	
    draw(infos[th_info->activeWin]);
    
    pthread_create(&th[0], NULL, (void *) drawThread, (void *) th_info);
    pthread_create(&th[1], NULL, (void *) inputThread, (void *) th_info);
    for(int i = 0; i < THREADS; i++) {
        pthread_join(th[i], NULL);
    }
    endwin();
    exit(0);
    return 0;
}

void inputThread(void *argv) 
{
    th_info_t *th = (th_info_t *) argv;
    for(;;) {
        char c = getch();
        pthread_mutex_lock(th->lock);
        manage_input(th->infos[th->activeWin]->file_name, th->infos[th->activeWin], c, th);
        pthread_mutex_unlock(th->lock);
    }
}

void drawThread(void *argv) 
{
    th_info_t *th = (th_info_t *) argv;
    struct timespec fps;
    fps.tv_sec = 0;
    fps.tv_nsec = 33333333.333; // about 30 fps
    for(;;) {
        nanosleep(&fps, NULL);
        pthread_mutex_lock(th->lock);
        for(int i = 0; i < th->windows; i++) {
            werase(th->infos[i]->win);
        }
        update_window_size(th);
        draw_all_screens(th);
        wrefresh(th->infos[th->activeWin]->win); // moves the cursor back to the active window
        pthread_mutex_unlock(th->lock);
    }
}

int create_win(th_info_t *th) 
{
    FILE *f;
    if(th->windows == 0) {
        th->infos[th->windows]->win = newwin(LINES, COLS, 0, 0);
        th->infos[th->windows]->winlines = LINES;
        th->infos[th->windows]->wincols = COLS;
        th->infos[th->windows]->x_threshold = th->infos[th->windows]->wincols * .5;
        th->infos[th->windows]->y_threshold = th->infos[th->windows]->winlines * .5;
        
        noecho();
        nodelay(th->infos[th->windows]->win, FALSE);
        keypad(th->infos[th->windows]->win, FALSE);
        scrollok(th->infos[th->windows]->win, FALSE);
        raw();
		
		parse_file(th->infos[th->windows]);
		
        f = fopen(th->infos[th->windows]->file_name, "r");
        if(f != NULL) {
            read_contents_from_file(f, th->infos[th->windows]);
        }
        if(strlen(current_line(th->infos[th->windows])) == 0) {
            current_line(th->infos[th->windows])[th->windows] = '\n';
        }
        th->windows++;
        return 1;
    } else {
        if(th->infos[th->activeWin]->wincols/2 <= 30) return 0;
        
        int beginX;
        if(th->activeWin == 0) beginX = 0;
        else beginX = th->infos[th->activeWin]->wincols;
        wresize(th->infos[th->activeWin]->win, th->infos[th->activeWin]->winlines, th->infos[th->activeWin]->wincols/th->windows);
        th->infos[th->activeWin]->wincols =  floor(th->infos[th->activeWin]->wincols / th->windows);
        th->infos[th->activeWin]->winlines = th->infos[th->activeWin]->winlines;
        th->infos[th->activeWin]->x_threshold = th->infos[th->activeWin]->wincols * .5;
        th->infos[th->activeWin]->y_threshold = th->infos[th->activeWin]->winlines * .5;
        
        th->infos[th->windows]->win = newwin(th->infos[th->activeWin]->winlines, th->infos[th->activeWin]->wincols, 0, th->infos[th->activeWin]->wincols+1+beginX);
        th->infos[th->windows]->wincols = th->infos[th->activeWin]->wincols;
        th->infos[th->windows]->winlines = th->infos[th->activeWin]->winlines;
        th->infos[th->windows]->x_threshold = th->infos[th->windows]->wincols * .5;
        th->infos[th->windows]->y_threshold = th->infos[th->windows]->winlines * .5;
        
		noecho();
		nodelay(th->infos[th->windows]->win, FALSE);
		keypad(th->infos[th->windows]->win, FALSE);
		scrollok(th->infos[th->windows]->win, FALSE);
		raw();
		
		parse_file(th->infos[th->windows]);
		
        f = fopen(th->infos[th->windows]->file_name, "r");
        if(f != NULL) {
            read_contents_from_file(f, th->infos[th->windows]);
        }
        if(strlen(current_line(th->infos[th->windows])) == 0) {
            current_line(th->infos[th->windows])[th->windows] = '\n';
        }
		th->windows++;
        return 1;
    }
}

int close_active_win(th_info_t *th) {
    if(th->activeWin >= th->windows) {
        return 0; // failed
    }
    werase(th->infos[th->activeWin]->win);
    wrefresh(th->infos[th->activeWin]->win);
    int addOn = floor(((float)th->infos[th->activeWin]->wincols)/(th->windows-1)); // how many cols to add to each window
    
    for(int j = th->activeWin; j < th->windows-1; j++) {
        th->infos[j] = th->infos[j + 1];
    }
    
    th->windows--;
    
    for(int i = 0; i < th->windows; i++) {
        th->infos[i]->wincols += addOn;
    }
    if(th->activeWin != 0) th->activeWin--;
    return 1;
}

void update_window_size(th_info_t *th)
{
    for(int i = 0; i < th->windows; i++) {
        int cols = th->infos[i]->wincols =  floor(COLS / th->windows);
        
        th->infos[i]->x_threshold = th->infos[i]->wincols * .5;
        th->infos[i]->y_threshold = th->infos[i]->winlines * .5;
        
        wresize(th->infos[i]->win, (float) th->infos[i]->winlines, cols);
        mvwin(th->infos[i]->win, (float) 0, cols*i);
        
    }
}
