#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "queue.h"


/*======================================================================*/
/* Data Structures */
/*======================================================================*/

struct queue
{
	struct Node *head, *tail;
	int size;
};

/* Linked list node */
struct Node
{
	void *val;
	struct Node *next;
};


/*======================================================================*/
/* Implementation */
/*======================================================================*/

/* Allocate memory for queue and initialize vars, return queue */
queue_t queue_create(void)
{
	struct queue *ret = malloc(sizeof(struct queue));
	ret->head = ret->tail = NULL;
	ret->size = 0;

	return ret;
}

/* Free queue memory */
int queue_destroy(queue_t queue)
{
	if (queue == NULL || queue->head)
	{
		return -1;
	}

	free(queue);

	return 0;
}

/* Enqueue data into queue */
int queue_enqueue(queue_t queue, void *data)
{
	struct Node *node = malloc(sizeof(struct Node)); // Allocate new node

	if (node == NULL)
	{
		return -1;
	}

	node->next = NULL; 	// Initialize and fill node
	node->val = data;

	// If not first node, just add to tail, else set head and tail
	if (queue->tail)
	{
		queue->tail->next = node;
		queue->tail = node;
	}
	else
	{
		queue->head = queue->tail = node;
	}

	queue->size += 1;

	return 0;
}

/* Dequeue next item from queue */
int queue_dequeue(queue_t queue, void **data)
{
	if (queue == NULL || queue->size == 0)
	{
		return -1;
	}

	struct Node *cur = queue->head; 	// Fill data parameter with node
	*data = cur->val;
	queue->head = queue->head->next;

	if (queue->head == NULL)
	{
		queue->tail = NULL;
	}

	free(cur);	// Free node memory and decrease size
	queue->size -= 1;

	return 0;
}

/* Delete first instance of data in queue */
int queue_delete(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL)
	{
		return -1;
	}

	struct Node *cur = queue->head, *prev = NULL; 	// Initialize to head of queue

	while (cur)
	{
		if (cur->val == data)
		{
			// If node is head, set head to next
			if (prev == NULL)
			{
				queue->head = cur->next;
			}

			// If node is tail, set tail to previous
			if (queue->tail == cur)
			{
				queue->tail = prev;
			}

			prev->next = cur->next;		// Remove node from list
			free(cur);					// Free memory and decrease size
			queue->size -= 1;

			return 0;
		}
		prev = cur;
		cur = cur->next;
	}
	free(cur);

	return -1;
}

/* Iterate through queue and apply function to each element */
int queue_iterate(queue_t queue, queue_func_t func)
{
	struct Node *cur = queue->head; 

	// Iterate and call func
	while (cur)
	{
		func(queue, cur->val);
		cur = cur->next;
	}

	return 0;
}

/* Return length of queue */
int queue_length(queue_t queue)
{
	return queue->size;
}



