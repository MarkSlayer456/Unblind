#ifndef MEMORY_H_

#define MEMORY_H_

void enlarge_lines_unblind_info(unblind_info_t *info);
void enlarge_characters_unblind_info(unblind_info_t *info, int y);
void setup_unblind_info(unblind_info_t *info);
void unblind_info_free(unblind_info_t *info);
void shutdown(WINDOW *win, unblind_info_t *info);
void reset_unblind_info_contents(unblind_info_t *info);

#endif