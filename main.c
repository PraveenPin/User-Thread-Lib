#include <stdio.h>
#include "my_pthread_t.h"
#include "queue.h"
#include <unistd.h>
#include <sys/time.h>


/***
 * busyWait - Function to mimic sleep() as sleep() wakes on alarm
 * @param 	i 	int 	Approx. duration of wait
 * @return 	null
 */
void busyWait(int i) {
	int j = 21474;
	i = i < 0 ? 1 : i;
	while (i>=0) {
		while (j>=0) {j--;}
		i--;
	}
}

void* thread1(void*g) {
	int i;
    for(i = 0; i < 10; i++){
        busyWait(1);
        printf("This is the first Thread 1\n");
     }
}

void thread2() {
    int i;
    for(i = 0; i < 3 ; i++) {
        busyWait(1);
        printf("This is the second Thread 2\n");
    }
    printf("Thread  2 EXITING!!!!!!!!\n");
    int someVal = 11;
    my_pthread_exit(&someVal);
}

void thread3() {
    int i;
    long j;
    for(i = 0; i < 2 ; i++) {
        busyWait(1);
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
        busyWait(1);
        printf("This is the fourth Thread 4\n");
    }
}

int main(int argc, const char * argv[]) {
	struct timeval start, end;
	float delta;
	gettimeofday(&start, NULL);
	my_pthread_t t1,t2,t3,t4;
    //Create threads
    my_pthread_create(&t1, NULL, &thread1,NULL);
    my_pthread_create(&t2, NULL, &thread2,NULL);
    my_pthread_create(&t3, NULL, &thread3,NULL);
    my_pthread_create(&t4, NULL, &thread4,NULL);
    gettimeofday(&end, NULL);
    delta = (((end.tv_sec  - start.tv_sec)*1000) + ((end.tv_usec - start.tv_usec)*0.001));
    printf("Execution time in Milliseconds: %f\n",delta);
    printf("Ending main!\n");
    return 0;
}