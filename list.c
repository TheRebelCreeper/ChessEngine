/**
 * Various list operations
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "bitboard.h"
#include "move.h"


// add a new move to the list of moves
void insert(struct node **head, Move newMove) {
	// add the new move to the list 
	struct node *newNode = malloc(sizeof(struct node));

	newNode->move = newMove;
	newNode->next = *head;
	*head = newNode;
}

// delete the selected move from the list
void delete(struct node **head, Move move) {
	struct node *temp;
	struct node *prev;

	temp = *head;
	// special case - beginning of list
	if (compareMoves(move, temp->move)) {
		*head = (*head)->next;
	}
	else {
		// interior or last element in the list
		prev = *head;
		temp = temp->next;
		while (!compareMoves(move, temp->move)) {
			prev = temp;
			temp = temp->next;
		}
		
		prev->next = temp->next;
	}
}

Node *getNode(struct node *head, int index)
{
	int i = 0;
	Node *temp = head;
	while (temp != NULL)
	{
		if (i == index)
			return temp;
		temp = temp->next;
		i++;
	}
	return temp;
}

void deleteList(struct node *head)
{
	Node *temp = head, *prev = NULL;
	while (temp != NULL)
	{
		prev = temp;
		temp = temp->next;
		free(prev);
	}
}
