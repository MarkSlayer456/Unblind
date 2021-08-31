#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include "unblind.h"
#include "csv_parse.h"
#include "messages.h"

void reset_unblind_info_contents(unblind_info_t *info) 
{
    for(int j = 0; j < info->max_lines-1; j++) {
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
    info->max_lines *= 2;
	info->contents = (char **)realloc(info->contents, info->max_lines * sizeof(char *));
	info->size = realloc(info->size, info->max_lines * sizeof(long long int));
	for(long long int i = (info->max_lines/2); i < info->max_lines; i++) {
		info->size[i] = DEFAULT_MAX_CHARS_PER_LINE;
        info->contents[i] = (char *)malloc(info->size[i] * sizeof(char));
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
	info->needs_saved = 0;
    info->prompt_save = 0;
	info->max_lines = DEFAULT_MAX_LINES;
// 	info->max_chars_per_line = DEFAULT_MAX_CHARS_PER_LINE;
    
	info->p_data = malloc(sizeof(parse_data_t));
	
    info->file_name = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
    info->cmd = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
    
    info->ur_manager = (undo_redo_manager_t *) malloc(sizeof(undo_redo_manager_t));
    setup_unblind_ur_manager(info->ur_manager);
    
    info->message = (char *)malloc(MAX_MESSAGE_LENGTH * sizeof(char));
    memset(info->message, 0, MAX_MESSAGE_LENGTH * sizeof(char));
    
    info->find = (d_linked_list_t *)malloc(sizeof(d_linked_list_t));
    info->find->head = NULL;
    info->fstr = (char *)malloc(sizeof(char) * FIND_STR_MAX_LENGTH);
    
    info->jstr = (char *)malloc(MAX_JUMP_STR_LENGTH * sizeof(char));
    
    memset(info->fstr, '\0', sizeof(char) * FIND_STR_MAX_LENGTH);
	info->contents = (char **)malloc(info->max_lines * sizeof(char *));
	info->size = malloc(info->max_lines * sizeof(long long int));
	for(int i = 0; i < info->max_lines; i++) {
		info->size[i] = DEFAULT_MAX_CHARS_PER_LINE;
		info->contents[i] = calloc(info->size[i], sizeof(char));
    }
}


void unblind_info_free_mini(unblind_info_t *info) 
{
    linked_list_d_free(info->ur_manager->stack_u, info->ur_manager->stack_u->head);
    linked_list_d_free(info->ur_manager->stack_r, info->ur_manager->stack_r->head);
    free(info->ur_manager->stack_u);
    free(info->ur_manager->stack_r);
    free(info->ur_manager);
    
    free(info->file_name);
    free(info->cmd);
    free(info->message);
	
	int size = 1024;
	if(info->p_data->words != NULL) {
		for(int i = 0; i < size; i++) {
			free(info->p_data->words[i]);
		}
		free(info->p_data->words);
		free(info->p_data->colors);
	}
	
	free(info->p_data);
    
	linked_list_d_free(info->find, info->find->head);
	free(info->find);
    
    free(info->fstr);
    free(info->jstr);
    
    for(int j = 0; j < info->max_lines; j++) {
        if(info->contents[j])
            free(info->contents[j]);
    }
    free(info->contents);
    free(info->size);
    
    free(info);
}

void parse_file(unblind_info_t *info) 
{
	language_t file_type = get_file_type(info);
	csv_data_t data;
	data.rows = 0;
	data.cols = 0;
	info->p_data->colorCount = 0;
	info->p_data->wordCount = 0;
	
	char *file = calloc(4096, sizeof(char));
	
	uid_t uid = getuid();
	struct passwd *pw;
	pw = getpwuid(uid);
	if(pw == NULL) {
		return;
	}
	strcat(file, pw->pw_dir);
	strcat(file, "/.unblind/language-syntax/");
	
	switch(file_type) {
		case C:
			strcat(file, C_CSV);
			data = parse(file);
			break;
		case JS:
			strcat(file, JS_CSV);
			data = parse(file);
			break;
		case PYTHON:
			strcat(file, PY_CSV);
			data = parse(file);
			break;
		case JAVA:
			strcat(file, JAVA_CSV);
			data = parse(file);
			break;
		case UNKNOWN:
			info->p_data->words = NULL;
			info->p_data->colors = NULL;
			free(file);
			return;
	}
	
	free(file);
	
	int size = 1024;
	info->p_data->words = calloc(size, sizeof(char *));
	for(int i = 0; i < size; i++) {
		info->p_data->words[i] = calloc(size, sizeof(char));
	}
	info->p_data->colors = calloc(size, sizeof(color_t));
	for(int i = 0; i < data.rows; i++) {
		for(int j = 0; j < data.cols; j++) {
			if(j == 0) {
				strcpy(info->p_data->words[info->p_data->wordCount], data.data[i][j]);
				info->p_data->wordCount++;
			} else {
				info->p_data->colors[info->p_data->colorCount] = get_color(data.data[i][j]); 
				info->p_data->colorCount++;
			}
		}
	}
}

void unblind_info_free(th_info_t *th) 
{
    for(int i = 0; i < th->windows; i++) {
        unblind_info_free_mini(th->infos[i]);
    }
    free(th->infos);
}

void shutdown(th_info_t *th) 
{
    endwin();
    unblind_info_free(th);
    free(th);
    exit(0);
}
