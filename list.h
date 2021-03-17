#ifndef List_H_
#define List_H

struct ListNode{
    int thread;
    struct ListNode *next;
};

typedef struct{
    struct ListNode *front;
    struct ListNode *back;
}List;

int addToList(int thread, List *list);

int isThisThreadInWaitingQueueForMutex(int id, List *waitingThreads);

void emptyList(List *waitingThreads);

void stateOfList(List *list);

//int removeFromList(List *List, int **thread);

//int isListEmpty(List *list);

//void stateOfList(List *list);

//void deleteAParticularListNodeFromList(List *list, int **thread);

#endif
