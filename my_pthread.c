// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#define _XOPEN_SOURCE 600
#include "my_pthread_t.h"
#include "queue.h"
#include <time.h>
#include <sys/time.h>
#include <setjmp.h>
#include <ucontext.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
// #include <threads.h>

int threadCount = 0;
int firstRun = 0;
int interruptsAreDisabled = 0;
int exitThreadIndex = 0;
static Queue queue[NUMBER_OF_LEVELS];
int exitedThreadIDs[50];
int threadLevelCount[NUMBER_OF_LEVELS];
static TCB* running;

// struct Queue waitQueue;
static int currentQueue;
static TCB *currentThread = 0;
static TCB *nextThread = 0;
static int id = 1;
struct itimerval tout_val;
int isSchedulerInitialized=0; //Call init only once-To initialize the schedular before anything else
long getCurrentTimestampNano();
long getCurrentTimestampSec();
void setAlarm(int seconds,suseconds_t microseconds);
static int waitingPriority=MAX_QUEUE_COUNT-1;
static int criticalSecTid=-1;
static int isYield= 0;
int *ids = 0;
long timeSinceLastMaintainance=0;
long thisQueueQuanta;

TCB * currentRunningThread;
TCB * tcbWaiting[50];
int f_or_c;
// ThreadFunc userFunction;

// sigset_t blocksigs;

// jmp_buf  schedContext;
// jmp_buf  sitBottomContext;

//swap context

TCB* Get_RunningThread(){
	return running;
}

void Start_Thread(void *(*start)(void *), void *arg)
{
	void *retVal = start((void *)arg);
	// TCB* running = Get_RunningThread();
	// running->retVal = retVal;
	my_pthread_exit(retVal);
}

void freeThread(TCB *threadToFree){
	if(threadToFree != NULL){
		free(threadToFree->stack);
		free(threadToFree->context);
		threadToFree = NULL;
	}
}

int findMaxPriorityQueue() {
	int i;
	for (i=0; i<MAX_QUEUE_COUNT; i++) {
		if (!isQueueEmpty(&queue[i])) {
			return i;
		}
	}
	return -1;
}

void scheduler(int signum){
	clock_gettime (CLOCK_REALTIME, (&(&queue[currentQueue])->front->thread->finish)); 
	cycle_counter++;
	//printf("Cycle cunter=%d\n",cycle_counter);
	sigset_t s;
	sigaddset(&s, SIGALRM);
	sigprocmask(SIG_BLOCK, &s, NULL);
	TCB *temp;	
	currentThread = (&queue[currentQueue])->front->thread;
	if(currentThread->firstCycle) {
		//~ printf("First cycle\n");
		currentThread->firstCycle=0;
	}
	else {
		double msecs, secs;
		secs=(double)(currentThread->finish.tv_sec-currentThread->start.tv_sec);
		if(secs==0) {	
			msecs=((double)(currentThread->finish.tv_nsec-currentThread->start.tv_nsec))/1000000;
		 }
		 else if(secs>=1) {
			secs=secs-1;
			msecs=((double)(999999999-(currentThread->start.tv_nsec))+(currentThread->finish.tv_nsec))/1000000;
		 }
		 currentThread->timeSpentSec += secs;
		 currentThread->timeSpentMsec += msecs;	
	}
	//~ printf("Time spent in secs %lf\n",currentThread->timeSpentSec);
	//~ printf("Time spent in Millisecs %lf\n",currentThread->timeSpentMsec);
	thisQueueQuanta=BASE_QUEUE_QUANTA * (currentQueue + 1);
	tout_val.it_value.tv_sec = 0; /* set timer for "INTERVAL (10) seconds */
	tout_val.it_value.tv_usec = thisQueueQuanta;
	tout_val.it_interval.tv_sec=0;
	tout_val.it_interval.tv_usec=0;
	removeFromQueue(&queue[currentQueue],&temp);
	timeSinceLastMaintainance=timeSinceLastMaintainance+thisQueueQuanta;
	if (currentThread->isFinished == 1) {
		int nextQueue= findMaxPriorityQueue();
		if(nextQueue != -1 ) {
			nextThread = queue[nextQueue].front->thread;
			// Update currentQueue value	
			currentQueue = nextQueue;
			currentThread->isCleaned = 1;		
			setitimer(ITIMER_REAL, &tout_val,0);
			setcontext(&(nextThread->context));
		} else {
			printf("All the queues are empty!!!!\n");
		}	
	} else {
			if ((isYield==1) && (currentThread->isWaiting)==1) {	
				(currentThread->yieldCount)++;
				addToQueue(temp, &queue);
				//~ printf("Removing from ready queue and adding to the wait queue\n");
			}
			else if ((isYield==1) && (currentThread->timeSpentMsec)<=0.002) {	
				(currentThread->yieldCount)++;
				addToQueue(temp, &queue[currentQueue]);
				//~ printf("Keeping thread %d it at the same priority\n",currentThread->id);
			}
			else {
				int addCurrentToQueue = currentQueue==2 ? 2 : currentQueue+1;
				if(addCurrentToQueue!=currentQueue) {
					currentThread->timeSpentMsec=0;
					currentThread->yieldCount=0;
				}
				addToQueue(temp, &queue[addCurrentToQueue ]);
				//~ printf("changing the priority to queue %d of thread %d\n", addCurrentToQueue, currentThread->id);
			}
			isYield=0;		
	}
	int nextQueue= findMaxPriorityQueue();	
	//printf("currentQueue: %d Current thread: %d\n",currentQueue, currentThread->id);
	currentQueue = nextQueue;
	nextThread = queue[nextQueue].front->thread;
	//printf("nextQueue: %d Next Thread: %d\n",nextQueue, nextThread->id);
	//printf("\n====================\n");
	setitimer(ITIMER_REAL, &tout_val,0);
	swapcontext(&(currentThread->context),&(nextThread->context));
	clock_gettime (CLOCK_REALTIME, (&(&queue[currentQueue])->front->thread->start)); 
	sigprocmask(SIG_UNBLOCK, &s, NULL);
}

/* create a new thread */
int my_pthread_create(TCB * tid, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	

	if(threadCount == 0){
		//creating main user thread
		TCB *thread = (TCB*)malloc(sizeof(TCB));
		if(thread == NULL){
			fprintf(stderr, "Failure to allocate memory for tmain thread");
			return -1;
		}
		thread->id = threadCount++;
		
		thread->context = (ucontext_t*)malloc(sizeof(ucontext_t));
		if(thread->context == NULL){
			fprintf(stderr, "Failure to allocate memory for thread context");
			return -1;
		}

		if(getcontext(thread->context) == -1){
			fprintf(stderr,"Failure to initialise execution context");
			return -1;
		}
		thread->waiting_id = -1;
		*ids = thread->id;
		thread->state = READY;
		thread->stack = thread->context->uc_stack.ss_sp;
		addToQueue(thread, &queue[0]);
		//set signal handler
		//init scheduler
		// scheduler();
		signal(SIGALRM, scheduler);

		printf("\n** First Thread Created**\n");
	}
	printf("\n** Later Threads Created**\n");

	TCB *thread = (TCB*)malloc(sizeof(TCB));
	if(thread == NULL){
		fprintf(stderr, "Failure to allocate memory for TCB of thread");
		return -1;
	}

	//Intialise TCB
	thread->context = (ucontext_t*)malloc(sizeof(ucontext_t));
	if(thread->context == NULL){
		fprintf(stderr, "Failure to allocate memory for thread context");
		return -1;
	}

	thread->state = READY;
	thread->id = threadCount++;
	thread->waiting_id = -1;

	thread->stack =  malloc(STACK_SIZE);
	if(thread->stack == NULL){
		fprintf(stderr,"Cannot allocate memory for stack");
		return -1;
	}

	if(getcontext(thread->context) == -1){
		fprintf(stderr,"Failure to initialise execution context");
		return -1;
	}

	thread->firstCycle = 1;
    thread->isFinished = 0;
    thread->isCleaned = 0;
    thread->timeSpentSec= 0;
    thread->timeSpentMsec= 0;
    thread->yieldCount=0;
    thread->start.tv_sec=0;
    thread->finish.tv_sec=0;
    thread->start.tv_nsec=0;
    thread->finish.tv_nsec=0;
    thread->isWaiting=0;
	thread->context->uc_stack.ss_sp = thread->stack;
	thread->context->uc_stack.ss_size = STACK_SIZE;
	thread->context->uc_stack.ss_flags = 0;
	thread->context->uc_link = 0;

	*ids = thread->id;

	addToQueue(thread, &queue[0]);
	printf("\n** Adding Thread %d to Queue: %p** \n", thread->id,queue[0].back);

	makecontext(thread->context, (void(*)(void))Start_Thread,2,function, arg);

	return thread->id;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	
	//call scheduler

	//if running thread's state is FINISHED then notify the waiting process 
	isYield = 1;
	signal(SIGALRM, scheduler);
	// raise(SIGALRM);
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	printf("\n** Before Exit**\n");
	TCB* running = Get_RunningThread();
	if(running->id == 0){ //main thread
		exit(0);
	}
	running->retVal = value_ptr;
	printf("Inside exit function -> value_ptr-> %d\n",*(int *)value_ptr);
	TCB* threadToFree = running;
	my_pthread_yield();
	 //if this causes prob, use global var
};

static void interruptDisable () {
	interruptsAreDisabled = 1;
}
static void interruptEnable () {
	interruptsAreDisabled = 0;
}

/* wait for thread termination */
int my_pthread_join(TCB thread, void **value_ptr) {
	//interrupt disable
	
	//interrupt enable
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	return 0;
};

