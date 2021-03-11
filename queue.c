#include<stdio.h>
#include<inttypes.h>
#include<stdlib.h>
#include<unistd.h>

#include "queue.h"
#include "my_pthread_t.h"

int addToQueue(TCB *thread, Queue *queue){
    if(queue->front == 0){
        queue->front = malloc(sizeof(struct Node));
        queue->front->thread = thread;
        queue->back = queue->front;
        queue->front->next = 0;
        queue->back->next = 0;
    }
    else{
        queue->back->next = malloc(sizeof(struct Node));
        queue->back = queue->back->next;
        queue->back->thread = thread;
        queue->back->next = 0;
    }
    
    return 1;
}



int removeFromQueue(Queue *queue, TCB **thread){
    if(queue->front == 0){
        return 0;
    }
    if(queue->front != queue->back){
        *thread = queue->front->thread;
        struct Node *tempNode = queue->front;
        queue->front = queue->front->next;
        free(tempNode);
    }
    else{
        *thread = queue->front->thread;
        free(queue->front);
        queue->front = 0;
        queue->back = 0;
    }
    return 1;
}

int isQueueEmpty(Queue *queue){
    if(queue->front == 0){
        return 1;
    }
    return 0;
}