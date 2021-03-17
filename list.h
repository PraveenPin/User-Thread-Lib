#ifndef List_H_
#define List_H

struct ListNode{
    int tid;
    struct ListNode *next;
};

typedef struct {
    struct ListNode *front;
    struct ListNode *back;
}TidQueue;

typedef struct {
    struct Node *front;
    struct Node *back;
}TCBQueue;

int addToTidQueue(int tid, TidQueue *waitingThreads);

int isThisThreadWaitingForMutex(int tid, TidQueue *waitingThreads);

void emptyTidQueue(TidQueue *waitingThreads);

void stateOfTidQueue(TidQueue *waitingThreads);

//int removeFromList(List *List, int **thread);

//int isListEmpty(List *list);

//void stateOfList(List *list);

//void deleteAParticularListNodeFromList(List *list, int **thread);

#endif
