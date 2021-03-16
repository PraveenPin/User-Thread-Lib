#include<stdio.h>
#include<inttypes.h>
#include<stdlib.h>
#include<unistd.h>

#include "list.h"


int addToList(int thread, List *queue){
    if(queue->front == 0){
        queue->front = malloc(sizeof(struct ListNode));
        queue->front->thread = thread;
        queue->back = queue->front;
        queue->front->next = 0;
        queue->back->next = 0;
    }
    else{
        queue->back->next = malloc(sizeof(struct ListNode));
        queue->back = queue->back->next;
        queue->back->thread = thread;
        queue->back->next = 0;
    }
    
    return 1;
}

int isThisThreadInWaitingQueueForMutex(int id, List *waitingThreads){
	struct ListNode* node = waitingThreads->front;
	while(node != NULL){
		if (node->thread == id){
			return 1;
		}
		node = node->next;
	}
	return 0;
}

void emptyList(List *waitingThreads){
    struct ListNode *tempNode = waitingThreads->front;
    while(tempNode != NULL){
        struct Node* buf=tempNode->next;
        free(tempNode);
        tempNode=buf;
    }
}
