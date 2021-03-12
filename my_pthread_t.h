// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server: 
#define _XOPEN_SOURCE 600
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <ucontext.h>
#include <sys/time.h>
// #include "queue.h"


#define STACK_SIZE 32768
#define NUMBER_OF_LEVELS 3
#define MAX_QUEUE_COUNT 3
#define BASE_QUEUE_QUANTA 50000//microseconds
#define THREAD_STACK (1024*1024) //size of the stack
#define MALLOC_ERROR 2

typedef enum{
	INIT,
	READY,
	RUNNING,
	SUSPENDED,
	BLOCKED,
	FINISHED
}my_pthread_state;

int cycle_counter;

typedef unsigned int my_pthread_t;

typedef unsigned long address_t;

typedef void (*th_executable_func)(void *arg);

typedef struct threadControlBlock {
	char my_pthread_name[31]; //name for a thread if required
	void *stack;
	ucontext_t *context; // context for this thread
	int isFinished;
    int isCleaned;
    int firstCycle;
    double timeSpentSec;
    double timeSpentMsec;
    struct timespec start, finish;
    int yieldCount;
    int isWaiting;
	int id; // thread id
	int priority; // thread priority
	my_pthread_state state; //thread state
	void *retVal; //return value from the function
	int waiting_id; //Thread id of the thread waiting on this thread
} TCB; 

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	/* add something here */
} my_pthread_mutex_t;

/* define your data structures here: */

// Feel free to add your own auxiliary data structures


/* Function Declarations: */

/* create a new thread */
int my_pthread_create(TCB * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(TCB thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif
