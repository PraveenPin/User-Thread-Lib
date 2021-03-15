#define _XOPEN_SOURCE 700
#include <stdio.h>
#include "my_pthread_t.h"
#include "queue.h"
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

my_pthread_mutex_t mutex;
/***
 * busyWait - Function to mimic sleep() as sleep() wakes on alarm
 * @param 	i 	int 	Approx. duration of wait
 * @return 	null
 */
void busyWait(int i) {
	int j = 21474;
	i = i < 0 ? 1 : i;
    int k = j*j;
    // k = k*k;
	while (k>0) {
        
        if(k%10000000 == 0) {printf("Inside Thread -> %d with value k=%d\n",i,k);}
		// sleep(5);
        // while (j>=0) {j--;}
		// i--;
        k--;
	}
    
}

int thread1(void*g) {
    printf("Thread 1 is trying to lock the mutex\n");
    my_pthread_mutex_lock(&mutex);
	int i;
    for(i = 0; i < 1; i++){
        busyWait(1);
        printf("This is the first Thread 1\n");
    }
    printf("Thread 1 is trying to release the mutex");
    my_pthread_mutex_unlock(&mutex);
    return 11;
}

void thread2() {
    int i;
    printf("Thread 2 is trying to lock the mutex \n");
    my_pthread_mutex_lock(&mutex);
    printf("Thread 2 has successfully acquired the lock");
    for(i = 0; i < 2 ; i++) {
        busyWait(2);
        printf("This is the second Thread 2\n");
    }
    printf("Thread 2 is trying to unlock the mutex");
    my_pthread_mutex_unlock(&mutex);
    printf("Thread  2 EXITING!!!!!!!!\n");
    int someVal = 11;
    my_pthread_exit(&someVal);
}

void thread3() {
    int i;
    long j;
    
    for(i = 0; i < 1 ; i++) {
        busyWait(3);
        printf("This is the third Thread 3\n");
    }
    for(i = 0; i < 4 ; i++) {
		for(j=0;j<1000000000;j++){}
        my_pthread_yield();
		printf("Thread 3 YIELDED!!\n");
    }
    
    printf("Thread  3 is done!\n");
}

void thread4() {
	int i;
    for(i = 0; i < 4 ; i++) {
        busyWait(4);
        printf("This is the fourth Thread 4\n");
    }
}

int main(int argc, const char * argv[]) {
	struct timeval start, end;
	float delta;
	gettimeofday(&start, NULL);
	my_pthread_t t1,t2,t3,t4;
    my_pthread_mutex_init(&mutex, NULL);
    int *retVal1, *retVal2, *retVal3, *retVal4;
    //Create threads
    my_pthread_create(&t1, NULL, &thread1,NULL);
    my_pthread_create(&t2, NULL, &thread2,NULL);
    my_pthread_create(&t3, NULL, &thread3,NULL);
    my_pthread_create(&t4, NULL, &thread4,NULL);
    my_pthread_join(t2, &retVal2);
    // my_pthread_join(t2, &retVal2);
    // my_pthread_join(t3, &retVal3);
    // my_pthread_join(t4, &retVal4);
    // printf("Retvals - %d %d %d %d\n",*retVal1,*retVal2,*retVal3, *retVal4);
    my_pthread_mutex_destroy(&mutex);
    gettimeofday(&end, NULL);
    delta = (((end.tv_sec  - start.tv_sec)*1000) + ((end.tv_usec - start.tv_usec)*0.001));
    printf("Execution time in Milliseconds: %f\n",delta);    
    printf("Ending main with return value of thread 1 -> %d\n",retVal1);
    return 0;
}
