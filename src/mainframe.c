#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "unblind.h"

void reset_unblind_info_contents(unblind_info_t *info) 
{
    for(int j = 0; j < MAX_LINES-1; j++) {
        if(info->contents[j] == NULL) break;
        for(int i = strlen(info->contents[j]); i < info->size[j]; ++i) {
            info->contents[j][i] = '\0';
        }
    }
}

void enlarge_characters_unblind_info(unblind_info_t *info, int y)
{
    info->size[y] *= 2;
    info->contents[y] = (char *) realloc(info->contents[y], info->size[y] * sizeof(char));
    memset(info->contents[y]+(info->size[y]/2), 0, ((info->size[y]/2) * sizeof(char)));
}

void enlarge_lines_unblind_info(unblind_info_t *info) 
{
    MAX_LINES *= 2;
    info->contents = (char **)realloc(info->contents, MAX_LINES * sizeof(char *));
    info->size = (int *) realloc(info->size, MAX_LINES * sizeof(int));
    for(int i = (MAX_LINES/2); i < MAX_LINES; i++) {
        info->size[i] = MAX_CHARS_PER_LINE;
        info->contents[i] = (char *)malloc(info->size[i] * sizeof(char));
//         info->contents[i] = (char *)realloc(info->contents[i], info->size[i] * sizeof(char));
        memset(info->contents[i], 0, info->size[i] * sizeof(char));
    }
}

void setup_unblind_info(unblind_info_t *info) 
{
    info->cx = 0;
    info->cy = 0;
    info->wcy = 0;
    info->wcx = 0;
    info->scroll_offset = 0;
    info->scrollX_offset = 0;
    info->m = EDIT;
    
    info->ur_manager = (undo_redo_manager_t *) malloc(sizeof(undo_redo_manager_t));
    setup_unblind_ur_manager(info->ur_manager);
    
    info->message = (char *)malloc(MAX_MESSAGE_LENGTH * sizeof(char));
    memset(info->message, 0, MAX_MESSAGE_LENGTH * sizeof(char));
    
    info->find = (d_linked_list_t *)malloc(sizeof(d_linked_list_t));
    info->fstr = (char *)malloc(sizeof(char) * FIND_STR_MAX_LENGTH);
    
    info->jstr = (char *)malloc(MAX_JUMP_STR_LENGTH * sizeof(char));
    
    memset(info->fstr, '\0', sizeof(char) * FIND_STR_MAX_LENGTH);
    info->contents = (char **)malloc(MAX_LINES * sizeof(char *));
    info->size = (int *) malloc(MAX_LINES * sizeof(int));
    for(int i = 0; i < MAX_LINES; i++) {
        info->size[i] = MAX_CHARS_PER_LINE;
        info->contents[i] = (char *)malloc(MAX_CHARS_PER_LINE * sizeof(char));
        memset(info->contents[i], 0, MAX_CHARS_PER_LINE * sizeof(char));
    }
}

void unblind_info_free(unblind_info_t *info) 
{
    
    linked_list_d_free(info->ur_manager->stack_u, info->ur_manager->stack_u->head);
    linked_list_d_free(info->ur_manager->stack_r, info->ur_manager->stack_r->head);
    free(info->ur_manager->stack_u);
    free(info->ur_manager->stack_r);
    free(info->ur_manager);
    
    free(info->message);
    
    linked_list_d_free(info->find, info->find->head);
    free(info->find);
    
    free(info->fstr);
    free(info->jstr);
    
    for(int i = 0; i < MAX_LINES; i++) {
        if(info->contents[i])
            free(info->contents[i]);
    }
    free(info->contents);
    free(info->size);
    
    free(info);
}

void shutdown(WINDOW *win, unblind_info_t *info) 
{
    endwin();
    unblind_info_free(info);
    exit(0);
}
