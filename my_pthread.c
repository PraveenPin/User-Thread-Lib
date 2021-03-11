// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include <sys/time.h>
#include<signal.h>

#include "my_pthread_t.h"
#include "queue.h"

int threadCount=0;
static Queue queue[NUMBER_OF_LEVELS];
static my_pthread_t* running;

my_pthread_t* Get_RunningThread(){
	return running;
}

void Start_Thread(void *(*start)(void *), void *arg)
{
	void *retVal = start((void *)arg);
	// my_pthread_t* running = Get_RunningThread();
	// running->retVal = retVal;
	my_pthread_exit(retVal);
}

void freeThread(my_pthread_t *threadToFree){
	if(threadToFree != NULL){
		free(threadToFree->stack);
		free(threadToFree->context);
		threadToFree = NULL;
	}
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
		
	if(threadCount == 0){
		//creating main user thread
		thread = (my_pthread_t*)malloc(sizeof(my_pthread_t));
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
		thread->state = READY;
		thread->stack = thread->context->uc_stack.ss_sp;
		addToQueue(thread, &queue[0]);
		//set signal handler
		//init scheduler
		printf("\n** First Thread Created**\n");
	}
	printf("\n** Later Threads Created**\n");

	thread = (my_pthread_t*)malloc(sizeof(my_pthread_t));
	if(thread == NULL){
		fprintf(stderr, "Failure to allocate memory for thread my_pthread_t");
		return -1;
	}

	//Intialise my_pthread_t
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

	addToQueue(thread, &queue[0]);
	printf("\n** Adding to Queue: %p** \n", queue[0].back);

	makecontext(thread->context, (void(*)(void))Start_Thread,2,function, arg);

	return thread->id;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	
	//call scheduler
	//if running thread's state is FINISHED then notify the waiting process 

	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	printf("\n** Before Exit**\n");
	my_pthread_t* running = Get_RunningThread();
	if(running->id == 0){ //main thread
		exit(0);
	}
	running->retVal = value_ptr;
	printf("Inside exit function -> value_ptr-> %d\n",*(int *)value_ptr);
	my_pthread_t* threadToFree = running;
	my_pthread_yield();
	freeThread(threadToFree); //if this causes prob, use global var
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	//
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

void interrupt_enable(void){
    sigset_t newSignal;
    sigemptyset(&newSignal);
    sigaddset(&newSignal, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &newSignal, NULL);
};

void interrupt_disable(void){
    sigset_t newSignal;
    sigemptyset(&newSignal);
    sigaddset(&newSignal, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &newSignal, NULL);
};

static void signal_handler(int signal_no){
    //my_pthread_yield();
    printf("signal handler get called ");
};


void interrupts_start(void){
    printf("interrupts");
    struct sigaction sa;
    struct itimerval it;
    
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGVTALRM, &sa, NULL)){
        fprintf(stderr, "Error in sigaction ");
        exit(1);
    }

    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 1000000/40;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 1000000/40;
    if(setitimer(ITIMER_VIRTUAL, &it, NULL)){
        fprintf(stderr, "error in setting up timer");
        exit(1);
    }
};
