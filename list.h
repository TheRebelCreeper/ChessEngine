#ifndef LIST_H
#define LIST_H
/**
 * list data structure containing the tasks in the system
 */

#include "move.h"

typedef struct node {
	Move move;
	struct node *next;
} Node;

// insert and delete operations.
void insert(struct node **head, Move move);
void delete(struct node **head, Move move);
Node *getNode(struct node *head, int index);
void deleteList(struct node *head);
#endif
