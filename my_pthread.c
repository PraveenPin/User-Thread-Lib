// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:
#define _XOPEN_SOURCE 700
#include <signal.h>
#include "my_pthread_t.h"
#include "queue.h"

#define NO_THREAD_ERROR -4
#define CANNOT_JOIN_ERROR -5
#define BASE_TIME_QUANTA 25

struct itimerval timer_val;
struct sigaction act_timer;
int threadCount=0;
static Queue queue[NUMBER_OF_LEVELS];
static Queue waitingQueue, finishedQueue;
static TCB* running;
static int threadIdForCriticalSection = -1;
static int isMutexLocked = 0;
static int totalCyclesElapsed = 0;
static long timeSinceLastMaintenance = 0;


TCB* findThreadById(my_pthread_t id, Queue *someQueue){
	struct Node* node = someQueue->front;
	while(node != NULL){
		if (node->thread->id == id){
			return node->thread;
		}
		node = node->next;
	}
	return NULL;
}

TCB* findThreadByIdInMLFQ(my_pthread_t id){
	int i;
	for (i = 0; i< NUMBER_OF_LEVELS; i++){
		struct Node *node = queue[i].front;
		while(node != NULL){
			if (node->thread->id == id){
				return node->thread;
			}
			node = node->next;		
		}
	}
	return NULL;
}

void enableInterrupts(void){
    sigset_t newSignal;
    sigemptyset(&newSignal);
    sigaddset(&newSignal, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &newSignal, NULL);
};

void disableInterrupts(void){
    sigset_t newSignal;
    sigemptyset(&newSignal);
    sigaddset(&newSignal, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &newSignal, NULL);
};

int findMaxPriorityQueue(){
	int i;
	for(i = 0;i < NUMBER_OF_LEVELS;i++){
		if(!isQueueEmpty(&queue[i])){
			return i;
		}
	}
	return -1;
}

void scheduleMaintenance(){
	printf("*********************** Calling Maintenance ***********************\n");
	//inherit code
	struct Node *prevNode = NULL,*tempNode = queue[NUMBER_OF_LEVELS - 1].front;
	while(tempNode != NULL){
		TCB* currentThread = tempNode->thread;
		if(currentThread->timeSpentInMilliSeconds > 250){
			currentThread->priority = 0;
			addToQueue(currentThread,&queue[currentThread->priority]);
			printf("Inverted Priority of thread %d to 0\n",currentThread->id);
			if(queue[NUMBER_OF_LEVELS - 1].back == queue[NUMBER_OF_LEVELS - 1].front){
				queue[NUMBER_OF_LEVELS - 1].front = 0;
				queue[NUMBER_OF_LEVELS - 1].back = 0;
			}
			else{
				if(tempNode == queue[NUMBER_OF_LEVELS - 1].back){
					queue[NUMBER_OF_LEVELS - 1].back = prevNode;
				}
				else if(tempNode == queue[NUMBER_OF_LEVELS - 1].front){
					queue[NUMBER_OF_LEVELS - 1].front = tempNode->next;
				}
				else{
					prevNode->next = tempNode->next;
				}
			}
		}
		else{
			prevNode = tempNode;
		}
		tempNode = tempNode->next;
	}

	printf("Threads in all queues\n");
	for(int i=0;i<NUMBER_OF_LEVELS;i++){
		stateOfQueue(&queue[i]);
	}
}

void initTimerInterrupt(int i){

	timer_val.it_value.tv_sec = 0;
	timer_val.it_value.tv_usec = i*1000;
	timer_val.it_interval.tv_usec= i*1000;
	timer_val.it_interval.tv_sec = 0;
	
	if(setitimer(ITIMER_VIRTUAL, &timer_val,NULL) == -1){
		fprintf(stderr,"Error calling setitimer for thread %d\n", running->id);
		printf("Error in Setting Timer\n");
	}
}

void scheduler(int sig){
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&(running->finish));
	// disable interrupts
	disableInterrupts();
	totalCyclesElapsed++;
	long currentTimeSlice = BASE_TIME_QUANTA*(running->priority + 1), nextTimeSlice = 0;

	if(running == NULL){
		printf("No running thread\n");
		exit(1);
	}
	if(running->firstCycle){
		running->firstCycle = 0;
	}
	else{
		double millisecs,secs;
		secs = (double)(running->finish.tv_sec - running->resume.tv_sec);
		if(secs == 0){
			millisecs = ((double)(running->finish.tv_nsec - running->resume.tv_nsec))/1000000;
		}
		else if(secs >= 1){
			secs = secs - 1;
			millisecs = ((double)((999999999 - running->resume.tv_nsec) + (running->finish.tv_nsec)))/1000000;
		}
		running->timeSpentInSeconds += secs;
		running->timeSpentInMilliSeconds += millisecs;
		running->totalTimeInSecs += running->timeSpentInSeconds;
		running->totalTimeInMilliSecs += running->timeSpentInMilliSeconds;
	}

	timeSinceLastMaintenance += currentTimeSlice;
	//check for maintenance
	if(timeSinceLastMaintenance >= 1000){
		//call maintenance cycle
		timeSinceLastMaintenance = 0;
		scheduleMaintenance();
	}

    
    if(isMutexLocked == 0) {
        printf("In if to shift threads from waiting queue to running queue that are waiting for mutex \n");
        isMutexLocked = 1;
        shiftFromWaitingToRunningQueue();
    }
	
	if(running->state == FINISHED){
		//we have to decide whther to free this or not
		// freeThread(oldThread);
		int nextPreferredQueue = findMaxPriorityQueue();
				
		printf("Time spent by exiting thread secs->%lf\tmillisec->%lf\n",running->totalTimeInSecs,running->totalTimeInMilliSecs);

		if(nextPreferredQueue == -1){
			printf("All the queue are empty\n");
		}
		else{
			TCB* nextThreadToRun;
			removeFromQueue(&queue[nextPreferredQueue],&nextThreadToRun);
			nextTimeSlice = BASE_TIME_QUANTA*(nextPreferredQueue + 1);
			if(nextThreadToRun->firstCycle == 1){
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&(nextThreadToRun->start));
			}
			printf("Swapping threads %d with %d\n", running->id, nextThreadToRun->id);
			running = nextThreadToRun;
			running->state = RUNNING;
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&(nextThreadToRun->resume));	
			initTimerInterrupt(nextTimeSlice);		
			setcontext(nextThreadToRun->context);
		}
	}
	else{	
		TCB* oldThread = running;
		if(oldThread->state == RUNNING){
			oldThread->state = READY;
			if(oldThread->timeSpentInMilliSeconds >= BASE_TIME_QUANTA*(oldThread->priority + 1)){
				// printf("Decreasing the priority of the thread %d\t to %d\n",oldThread->id,oldThread->priority);
				//push to low level queue
				if(oldThread->priority != (NUMBER_OF_LEVELS - 1)){
					oldThread->priority+=1;
					oldThread->timeSpentInSeconds = 0;
					oldThread->timeSpentInMilliSeconds = 0;					
				}
			}
			addToQueue(oldThread,&queue[oldThread->priority]);

			if(isQueueEmpty(&queue[oldThread->priority])){
				printf("Problem with the queue or thread\n ");
				fprintf(stderr,"Problem with adding current thread to ready queue\n");
			}
		}
		stateOfQueue(&queue[oldThread->priority]);

		int nextPreferredQueue = findMaxPriorityQueue();

		if(nextPreferredQueue == -1){
			printf("All the queue are empty\n");
		}
		else{
			TCB* nextThreadToRun;
			removeFromQueue(&queue[nextPreferredQueue],&nextThreadToRun);
			nextTimeSlice = BASE_TIME_QUANTA*(nextPreferredQueue + 1);
			if(nextThreadToRun->firstCycle == 1){
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&(nextThreadToRun->start));
			}
			printf("Swapping threads %d with %d\n", oldThread->id, nextThreadToRun->id);
			running = nextThreadToRun;
			running->state = RUNNING;
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&(nextThreadToRun->resume));
			initTimerInterrupt(nextTimeSlice);
			if(swapcontext(oldThread->context, nextThreadToRun->context) == -1){
				fprintf(stderr,"Insufficient stack space left\n");
			}
		}
		
	}
	enableInterrupts();
}

void setupSignal(){
	act_timer.sa_handler= scheduler;
	sigemptyset(&act_timer.sa_mask);
	act_timer.sa_flags = SA_RESTART; //0
	sigemptyset(&act_timer.sa_mask);
	sigaddset(&act_timer.sa_mask,SIGVTALRM);
	if(sigaction(SIGVTALRM,&act_timer,NULL)){
		fprintf(stderr, "Error in sigaction \n");
		printf("************** Error in sigaction ************** \n");
		exit(1);
	}
}	

void Start_Thread(void *(*start)(void *), void *arg){
	void *retVal = start((void *)arg);
	my_pthread_exit(retVal);
}

void freeThread(TCB *threadToFree){
	printf("Freeing up space of thread - %d\n",threadToFree->id);
	if(threadToFree != NULL){
		free(threadToFree->stack);
		free(threadToFree->context);
		threadToFree = NULL;
	}
}

/* create a new thread */
int my_pthread_create(my_pthread_t * tid, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
		
	if(threadCount == 0){
		//creating main user thread
		TCB *mainThread = (TCB*)malloc(sizeof(TCB));
		if(mainThread == NULL){
			fprintf(stderr, "Failure to allocate memory for main thread\n");
			return -1;
		}
		mainThread->id = threadCount++;
		
		mainThread->context = (ucontext_t*)malloc(sizeof(ucontext_t));
		if(mainThread->context == NULL){
			fprintf(stderr, "Failure to allocate memory for mainThread context\n");
			return -1;
		}

		if(getcontext(mainThread->context) == -1){
			fprintf(stderr,"Failure to initialise execution context\n");
			return -1;
		}
		mainThread->waiting_id = -1;
		*tid = mainThread->id;
		mainThread->state = RUNNING;
		mainThread->stack = mainThread->context->uc_stack.ss_sp;
		mainThread->firstCycle = 1;
		mainThread->priority = 0;
		mainThread->timeSpentInMilliSeconds = 0;
		mainThread->timeSpentInSeconds = 0;
		mainThread->totalTimeInMilliSecs = 0;
		mainThread->totalTimeInSecs = 0;
		mainThread->created.tv_sec = 0;
		mainThread->created.tv_nsec = 0;
		mainThread->start.tv_sec = 0;
		mainThread->start.tv_nsec = 0;
		mainThread->resume.tv_sec = 0;
		mainThread->resume.tv_nsec = 0;
		mainThread->finish.tv_sec = 0;
		mainThread->finish.tv_nsec = 0;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&(running->created));
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&(running->start));
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&(running->resume));
		running = mainThread;
		printf("Main Thread\n");

		//setting the timer interrupt and signal handler
		setupSignal();
		initTimerInterrupt(BASE_TIME_QUANTA);
	}

	TCB *thread = (TCB*)malloc(sizeof(TCB));
	if(thread == NULL){
		fprintf(stderr, "Failure to allocate memory for TCB of thread\n");
		return -1;
	}

	//Intialise TCB
	thread->context = (ucontext_t*)malloc(sizeof(ucontext_t));
	if(thread->context == NULL){
		fprintf(stderr, "Failure to allocate memory for thread context\n");
		return -1;
	}

	thread->state = READY;
	thread->id = threadCount++;
	thread->waiting_id = -1;
	thread->firstCycle = 1;
	thread->priority = 0;
	thread->timeSpentInMilliSeconds = 0;
	thread->timeSpentInSeconds = 0;
	thread->totalTimeInMilliSecs = 0;
	thread->totalTimeInSecs = 0;
	thread->created.tv_sec = 0;
	thread->created.tv_nsec = 0;
	thread->start.tv_sec = 0;
	thread->start.tv_nsec = 0;
	thread->resume.tv_sec = 0;
	thread->resume.tv_nsec = 0;
	thread->finish.tv_sec = 0;
	thread->finish.tv_nsec = 0;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&(running->created));

	thread->stack =  malloc(STACK_SIZE);
	if(thread->stack == NULL){
		fprintf(stderr,"Cannot allocate memory for stack\n");
		return -1;
	}

	if(getcontext(thread->context) == -1){
		fprintf(stderr,"Failure to initialise execution context\n");
		return -1;
	}

	thread->context->uc_stack.ss_sp = thread->stack;
	thread->context->uc_stack.ss_size = STACK_SIZE;
	thread->context->uc_stack.ss_flags = 0;
	thread->context->uc_link = 0;

	*tid = thread->id;

	addToQueue(thread, &queue[thread->priority]);
	printf("Front thread at queue %d\n",queue[thread->priority].back->thread->id);
	printf("\n** Adding Thread %d to Queue: %p** \n", thread->id,queue[thread->priority].back);

	makecontext(thread->context, (void(*)(void))Start_Thread,2,function, arg);

	return thread->id;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {

	disableInterrupts();
	printf("Thread %d is giving up CPU state: %d\n",running->id, running->state);
	
	if(running->state == WAITING){
		printf("Adding the current thread to wait queue\n");
		//add to waiting list
		addToQueue(running,&waitingQueue);		
	}
	else if( running->state == FINISHED){
		printf("Adding the current finished thread to finished queue\n");
		addToQueue(running,&finishedQueue);
	}
	printf("Raising a signal to call scheduler\n");
	enableInterrupts();
	raise(SIGVTALRM);	
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	disableInterrupts();
	printf("\n** Just Before Exit of thread %d with return value %d**\n",running->id, (int)value_ptr);
	if(running->id == 0){ //main thread
		exit(0);
	}
	running->state = FINISHED;
	if(running->waiting_id >= 0){
		if(value_ptr != NULL){
			*running->retVal = &value_ptr;
		}
		//loop the waiting Queue for thread and set state to ready
		TCB* waitingThread;
		//deleting the waiting thread in waiting queue and adding into ready queue
		deleteAParticularNodeFromQueue(running->waiting_id,&waitingQueue,&waitingThread);
		if(waitingThread == NULL){
			printf("Some problem in accessing waiting thread %d\n", running->waiting_id);
			//call scheduler
			exit(1); //add some error
		}
		running->waiting_id = -1;
		// stateOfQueue(&waitingQueue);
		waitingThread->state = READY;
		addToQueue(waitingThread,&queue[waitingThread->priority]);
		stateOfQueue(&queue[waitingThread->priority]);
	}
	my_pthread_yield();
	enableInterrupts();
	 //if this causes prob, use global var
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t tid, void **value_ptr) {
	//interrupt disable
	disableInterrupts();
	if(tid > MAX_THREADS){
		return NO_THREAD_ERROR;
	}
	printf("Searching in all queues\n");
	TCB* threadToWaitOn = findThreadByIdInMLFQ(tid);
	if(threadToWaitOn == NULL){
		threadToWaitOn = findThreadById(tid, &waitingQueue);
		if(threadToWaitOn == NULL){
			threadToWaitOn = findThreadById(tid, &finishedQueue);
			if(threadToWaitOn == NULL){
				fprintf(stderr,"No thread with id %d to wait on in any queue\n",tid);
				return NO_THREAD_ERROR;
			}
			else{
				stateOfQueue(&finishedQueue);
				if(threadToWaitOn->retVal != NULL){
					*value_ptr = *threadToWaitOn->retVal;
				}
				return 0;
			}
		}
	}
	if(running == NULL){
		fprintf(stderr,"Problem with accessing current thread\n");
		return -1;
	}
	if(running->id == tid){
		fprintf(stderr,"Cannot wait on itself\n");
		return -1;
	}
	printf("Thread %d started waiting on thread %d\n",running->id,threadToWaitOn->id);
	if(threadToWaitOn->waiting_id >= 0 ){
		fprintf(stderr,"Some other thread has joined this thread %d\n",tid);
		return CANNOT_JOIN_ERROR;
	}
	threadToWaitOn->waiting_id = running->id;
	threadToWaitOn->retVal = value_ptr;
	running->state = WAITING;
	my_pthread_yield();
	//interrupt enable
	enableInterrupts();
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	mutex = ( my_pthread_mutex_t *) malloc(sizeof( my_pthread_mutex_t));
	if(mutex == NULL){
		printf("Error allocating memory for Mutex block \n");
		return -1;
	}
	if(mutex->isLocked == 1)
		return -1;
	
    mutex->isLocked = 0;
    mutex->mutexattr = mutexattr;
    printf("Mutex created");
    return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	printf("Check is mutex locked? %d\n", mutex -> isLocked);
    while(1){
        if(mutex -> isLocked == 1) {
            running -> mutex_acquired_thread_id = threadIdForCriticalSection; 
            running->state = WAITING;
            my_pthread_yield();
            return 0;
        }
        else {
            printf("Mutex is not locked, locking mutex by thread %d\n", running ->id );
            mutex -> isLocked = 1;
            setThreadIdForCritialSection();
            return 0;
        }
    }  
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
    if(mutex == NULL){
        my_pthread_mutex_init(&mutex, NULL);
    }
    
    if(mutex->isLocked == 1) {
        mutex->isLocked = 0;
        unlockTheMutex();
    }
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	my_pthread_mutex_unlock(&mutex);
    mutex = NULL;
    return 0;
};

int setThreadIdForCritialSection() {
    printf("set thread Id for critial section\n");
    threadIdForCriticalSection = running -> id;
    printf("thread id for critical section is %d \n", threadIdForCriticalSection);
    if(threadIdForCriticalSection == -1 ) {
        fprintf(stderr,"Some error occurred in allocating mutex to thread");
        return -1;
    }
    isMutexLocked = 1;
}

int unlockTheMutex(){
    isMutexLocked = 0;
    return 0;
}

void shiftFromWaitingToRunningQueue() {
    struct Node *tempNode = waitingQueue.front;
    struct Node *prevNode = NULL;
    
    while(tempNode != NULL){
        if(tempNode -> thread -> mutex_acquired_thread_id == threadIdForCriticalSection){
            TCB *tempThread = tempNode -> thread;
            stateOfQueue(&waitingQueue);
            if(waitingQueue.back == waitingQueue.front){
				waitingQueue.front = 0;
				waitingQueue.back = 0;
			}
			else{
				if(tempNode == waitingQueue.back){
					waitingQueue.back = prevNode;
                    waitingQueue.back -> next = NULL;
				}
				else if(tempNode == waitingQueue.front){
					waitingQueue.front = tempNode->next;
				}
				else{
					prevNode->next = tempNode->next;
				}
			}
            //deleteAParticularNodeFromQueue(tempNode -> thread->id, &waitingQueue, &tempThread);
            addToQueue(tempThread, &queue[tempThread -> priority]);
        }
        else{
			prevNode = tempNode;
		}
        tempNode = tempNode->next;  
    }
}
