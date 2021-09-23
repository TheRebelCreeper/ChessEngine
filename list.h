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
void traverse(struct node *head);
