#define _XOPEN_SOURCE 700
#include <stdio.h>
#include "my_pthread_t.h"
#include "queue.h"
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

my_pthread_mutex_t mutex, mutex2;
/***
 * busyWait - Function to mimic sleep() as sleep() wakes on alarm
 * @param 	i 	int 	Approx. duration of wait
 * @return 	null
 */
void busyWait(int i) {
	int j = 21474;//21474
	i = i < 0 ? 1 : i;
    int k = j*j;
	while (k>0) {
        // if(k%10000000 == 0) {printf("Inside Thread -> %d with value k=%d\n",i,k);
		// sleep(5);
        // while (j>=0) {j--;}
		// i--;
        k--;
	}
    
}

int thread1(void*g) {
    printf("Thread 1 is trying to lock the mutex\n");
    my_pthread_mutex_lock(&mutex);
    my_pthread_mutex_lock(&mutex2);
	int i;
    for(i = 0; i < 2; i++){
        busyWait(1);
        my_pthread_mutex_unlock(&mutex);
        //printf("This is the first Thread 1\n");
    }
    my_pthread_mutex_unlock(&mutex2);
    return 11;
}

void thread2() {
    int i;
    printf("Thread 2 is trying to lock the mutex \n");
    my_pthread_mutex_lock(&mutex);
    printf("Thread 2 has successfully acquired the lock\n");
    for(i = 0; i < 2 ; i++) {
        busyWait(2);
        //printf("This is the second Thread 2\n");
    }
    printf("Thread 2 is trying to unlock the mutex\n");
    my_pthread_mutex_unlock(&mutex);
    printf("Thread  2 EXITING!!!!!!!!\n");
}

int thread3() {
    int i;
    my_pthread_mutex_lock(&mutex);
    my_pthread_mutex_lock(&mutex2);

    for(i = 0; i < 3 ; i++) {
        busyWait(3);
        //printf("This is the third Thread 3\n");
        my_pthread_mutex_unlock(&mutex2);
    }
    my_pthread_mutex_unlock(&mutex);

    printf("Thread  3 is done!\n");
    return 777;

}

void thread4() {
	int i;
    my_pthread_mutex_lock(&mutex2);
    for(i = 0; i < 3 ; i++) {
        busyWait(4);
        //printf("This is the fourth Thread 4\n");
    }
    my_pthread_mutex_unlock(&mutex2);
}

int main(int argc, const char * argv[]) {
	struct timeval start, end;
	float delta;
	gettimeofday(&start, NULL);
	my_pthread_t t1,t2,t3,t4;
    my_pthread_mutex_init(&mutex, NULL);
    my_pthread_mutex_init(&mutex2, NULL);
    int *retVal1, *retVal2, *retVal3, *retVal4;
    //Create threads
    my_pthread_create(&t1, NULL, &thread1,NULL);
    my_pthread_create(&t2, NULL, &thread2,NULL);
    my_pthread_create(&t3, NULL, &thread3,NULL);
    my_pthread_create(&t4, NULL, &thread4,NULL);

    my_pthread_join(t1, &retVal1);
    my_pthread_join(t2, &retVal2);
    my_pthread_join(t3, &retVal3);
    my_pthread_join(t4, &retVal4);
    // printf("Retvals - %d %d %d %d\n",*retVal1,*retVal2,*retVal3, *retVal4);
    
    my_pthread_mutex_destroy(&mutex);
    my_pthread_mutex_destroy(&mutex2);
    gettimeofday(&end, NULL);
    delta = (((end.tv_sec  - start.tv_sec)*1000) + ((end.tv_usec - start.tv_usec)*0.001));
    printf("Execution time in Milliseconds: %f\n",delta);    
    printf("Ending main\n");
    return 0;
}
