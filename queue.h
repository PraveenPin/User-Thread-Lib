#ifndef QUEUE_H_
#define QUEUE_H

#include "my_pthread_t.h"

struct Node{
    my_pthread_t *thread;
    struct Node *next;
};

typedef struct{
    struct Node *front;
    struct Node *back;
}Queue;

int addToQueue(my_pthread_t* thread, Queue *queue);

int removeFromQueue(Queue *queue, my_pthread_t **thread);

int isQueueEmpty(Queue *queue);

#endif