// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:
#define _XOPEN_SOURCE 700
#include <sys/time.h>
#include <signal.h>
#include "my_pthread_t.h"
#include "queue.h"


#define NO_THREAD_ERROR -4

struct itimerval timer_val;
struct sigaction act_timer;
int threadCount=0;
static Queue queue[NUMBER_OF_LEVELS];
static Queue waitingQueue, finishedQueue;
static TCB* running;
static int threadIdForCriticalSection = -1;
static int isMutexLocked = 0;

TCB* findThreadById(my_pthread_t id, Queue *someQueue){
	struct Node* node = someQueue->front;
	while(node != NULL){
		printf("In queue :%d\n",node->thread->id);
		if (node->thread->id == id){
			break;
		}
		node = node->next;
	}
	return node->thread;
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

void scheduler(int sig){
	// disable interrupts
	disableInterrupts();

	if(running == NULL){
		printf("No running thread");
		exit(1);
	}
	
	TCB* oldThread = running;
	if(oldThread->state == RUNNING){
		oldThread->state = READY;
		addToQueue(oldThread,&queue[0]);

		if(isQueueEmpty(&queue[0])){
			printf("Problem with the queue or thread\n ");
			fprintf(stderr,"Problem with adding current thread to ready queue");
		}
	}
    
    if(isMutexLocked == 0) {
        printf("In if to shift threads from waiting queue to running queue that are waiting for mutex \n");
        isMutexLocked = 1;
        shiftFromWaitingToRunningQueue();
    }
	stateOfQueue(&queue[0]);
    stateOfQueue(&waitingQueue);
	TCB* nextThreadToRun;
	removeFromQueue(&queue[0],&nextThreadToRun);
	printf("Swapping threads %d with %d\n", oldThread->id, nextThreadToRun->id);
	running = nextThreadToRun;
	running->state = RUNNING;
	if(oldThread->state == FINISHED){
		//we have to decide whther to free this or not
	}

	swapcontext(oldThread->context, nextThreadToRun->context);
	enableInterrupts();
}

void initTimerInterrupt(){

	act_timer.sa_handler= scheduler;
	sigemptyset(&act_timer.sa_mask);
	act_timer.sa_flags = SA_RESTART; //0
	sigemptyset(&act_timer.sa_mask);
	sigaddset(&act_timer.sa_mask,SIGVTALRM);
	if(sigaction(SIGVTALRM,&act_timer,NULL)){
		fprintf(stderr, "Error in sigaction ");
		exit(1);
	}
	
	timer_val.it_value.tv_sec = 0;
	timer_val.it_value.tv_usec = 10000000/40;
	timer_val.it_interval.tv_usec= 1000000/40;
	timer_val.it_interval.tv_sec = 0;

	if(setitimer(ITIMER_VIRTUAL, &timer_val,NULL) == -1){
		fprintf(stderr,"Error calling setitimer for thread %d\n", running->id);
		printf("Error in Setting Timer");
	}
}

void Start_Thread(void *(*start)(void *), void *arg){
	void *retVal = start((void *)arg);
	// running->retVal = retVal;
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
			fprintf(stderr, "Failure to allocate memory for main thread");
			return -1;
		}
		mainThread->id = threadCount++;
		
		mainThread->context = (ucontext_t*)malloc(sizeof(ucontext_t));
		if(mainThread->context == NULL){
			fprintf(stderr, "Failure to allocate memory for mainThread context");
			return -1;
		}

		if(getcontext(mainThread->context) == -1){
			fprintf(stderr,"Failure to initialise execution context");
			return -1;
		}
		mainThread->waiting_id = -1;
		*tid = mainThread->id;
		mainThread->state = RUNNING;
		mainThread->stack = mainThread->context->uc_stack.ss_sp;
		running = mainThread;
		printf("Main Thread");

		//setting the timer interrupt and signal handler
		initTimerInterrupt();
	}

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

	thread->context->uc_stack.ss_sp = thread->stack;
	thread->context->uc_stack.ss_size = STACK_SIZE;
	thread->context->uc_stack.ss_flags = 0;
	thread->context->uc_link = 0;

	*tid = thread->id;

	addToQueue(thread, &queue[0]);
	printf("Front thread at queue %d\n",queue[0].back->thread->id);
	printf("\n** Adding Thread %d to Queue: %p** \n", thread->id,queue[0].back);

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
		//loop the waiting Queue for thread and set state to ready
		TCB* waitingThread;
		//deleting the waiting thread in waiting queue and adding into ready queue
		deleteAParticularNodeFromQueue(running->waiting_id,&waitingQueue,&waitingThread);
		if(waitingThread == NULL){
			printf("Some problem in accessing waiting thread %d\n", running->waiting_id);
			//call scheduler
			exit(1);
		}
		// stateOfQueue(&waitingQueue);
		waitingThread->state = READY;
		int value = (int)value_ptr;
		running->retVal = &value;
		addToQueue(waitingThread,&queue[0]);
		stateOfQueue(&queue[0]);
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

	TCB* threadToWaitOn = findThreadById(tid, &queue[0]);
	if(threadToWaitOn == NULL){
		threadToWaitOn = findThreadById(tid, &finishedQueue);
		if(threadToWaitOn == NULL){
			threadToWaitOn = findThreadById(tid, &waitingQueue);
			if(threadToWaitOn == NULL){
				fprintf(stderr,"No thread with id %d to wait on in any queue\n",tid);
				return NO_THREAD_ERROR;
			}
		}
	}
	if(running == NULL){
		fprintf(stderr,"Problem with accessing current thread\n");
		return -1;
	}
	printf("Thread %d started waiting on thread %d\n",running->id,threadToWaitOn->id);
	threadToWaitOn->waiting_id = running->id;
	*value_ptr = threadToWaitOn->retVal;
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
        printf("inside while %d \n", mutex -> isLocked);
        if(mutex -> isLocked == 1) {
            printf("Started process to move running thread to waiting queue\n");
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

int shiftFromWaitingToRunningQueue() {
    printf("In shift from waiting to running queue function\n");
    struct Node *tempNode = waitingQueue.front;
    struct TCB *temp = waitingQueue.front->thread;
    while(tempNode != NULL){
        printf("shiftFromWaitingToRunningQueue: %d\t\n",tempNode->thread->id);
        printf("threadIdForCriticalSection %d\n", threadIdForCriticalSection);
        printf("waiting thread for current thread %d\n",tempNode -> thread -> mutex_acquired_thread_id );
        if(tempNode -> thread -> mutex_acquired_thread_id == threadIdForCriticalSection){
            printf("Inside thread removal and addition %d\n ", temp);
            removeFromQueue(&waitingQueue, &temp);
            stateOfQueue(&waitingQueue);
            addToQueue(tempNode -> thread, &queue[0]);
        }
        tempNode = tempNode->next;
    }
    stateOfQueue(&waitingQueue);
    stateOfQueue(&queue[0]);
    printf("\n");
    return 0;
}
