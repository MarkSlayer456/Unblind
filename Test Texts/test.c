/**
* This file is used for test programming with unblind
* the code in this file is just for fun and shouldn't be taken seriously
* this file also may not work all the time
**/

#include <stdio.h>
#include <stdlib.h>

typedef struct node {
	int value;
	struct node *left, *right;
} node_t;

typedef struct btree {
	node_t *root;
} btree_t;

btree_t *btree_new(int value);
node_t *node_new(int value);
void node_insert(node_t *node, btree_t *tree);

void getGood(int n);

int main(int argc, char *argv[]) {
	int x;
	scanf("%d", &x);
	getGood(x);
	btree_t *main_tree = btree_new(5);
	node_t *node1 = node_new(17);
	node_t *node2 = node_new(23);
	node_t *node3 = node_new(2);
	node_insert(node1, main_tree);
	node_insert(node2, main_tree);
	node_insert(node3, main_tree);
	printf("%d\n",main_tree->root->value);
	return 0;
}


void getGood(int n) {
	for(int i = 0; i < n; i++) {
		printf("Get good!\n");
	}
}

void node_insert(node_t *node, btree_t *tree) {
	node_t *curr = tree->root;
	while(curr) {
		if(curr->value > node->value) {
			if(curr->left) {
				curr = curr->left;
				continue;
			} else {
				curr->left = node;
				return;
			}
		} else {
			if(curr->right) {
				curr = curr->right;
				continue;
			} else {
				curr->right = node;
				return;
			}
		}
	} 
	if(!curr) {
		fprintf(stderr, "Tree is NULL\n");
	}
}

node_t *node_new(int value) {
	node_t *node = (node_t *) malloc(sizeof(node_t));
	node->value = value;
	node->right = NULL;
	node->left = NULL;
	return node;
}

btree_t *btree_new(int value) {
	node_t *node = node_new(value);
	btree_t *tree = (btree_t *) malloc(sizeof(btree_t));
	tree->root = node;
	return tree;
}