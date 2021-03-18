#ifndef DOUBLE_LINKED_LIST_

#define DOUBLE_LINKED_LIST_

typedef enum {
	TYPE = 1,
	BACKSPACE = 2,
	BACKSPACE_LAST_CHAR = 3,
	ENTER = 4,
	ENTER_MIDDLE_OF_LINE = 5,
	TAB = 6,
	DELETE_LINE = 7,
	DUP_LINE = 8
} action_t;

typedef struct ur_node {
    char *c;
    action_t action;
} ur_node_t;

typedef struct dll_node {
	int x;
	int y;
	void *value;
	struct dll_node *next, *prev;
} dll_node_t;

typedef struct d_linked_list {
	dll_node_t *head, *tail;
	int curr;
} d_linked_list_t;

typedef struct undo_redo_manager {
	d_linked_list_t *stack_u;
	d_linked_list_t *stack_r;
} undo_redo_manager_t;

d_linked_list_t *linked_list_d_create();
void linked_list_d_add(d_linked_list_t *dll, void *value, int x, int y);
dll_node_t *linked_list_d_get(d_linked_list_t *dll, int i);
dll_node_t *linked_list_d_pop(d_linked_list_t *dll);

void linked_list_d_free(d_linked_list_t *dll, dll_node_t *node);

void setup_unblind_ur_manager(undo_redo_manager_t *ur_manager);


#endif
