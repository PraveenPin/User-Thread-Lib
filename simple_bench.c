#include <stdio.h>
#include "my_pthread_t.h"
#include "queue.h"
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

/***
 * busyWait - Function to mimic sleep() as sleep() wakes on alarm
 * @param 	i 	int 	Approx. duration of wait
 * @return 	null
 */
void busyWait(int i) {
	int j = 21474;
	i = i < 0 ? 1 : i;
    int k = j*j;
	while (k>0) {
        k--;
	}
    
}

int threadFunc1(void*g) {
	int i;
    for(i = 0; i < 2; i++){
        busyWait(1);
        //printf("This is the first Thread 1\n");
    }
    return 11;
}

void threadFunc2() {
    int i;
    for(i = 0; i < 2 ; i++) {
        busyWait(2);
        //printf("This is the second Thread 2\n");
    }
    printf("Thread  2 EXITING!!!!!!!!\n");
}

int threadFunc3() {
    int i;
    
    for(i = 0; i < 3 ; i++) {
        busyWait(3);
        //printf("This is the third Thread 3\n");
    }
    printf("Thread  3 is done!\n");
    return 777;

}

void threadFunc4() {
	int i;
    for(i = 0; i < 3 ; i++) {
        busyWait(4);
        //printf("This is the fourth Thread 4\n");
    }
}

int main(int argc, const char * argv[]) {
	struct timeval start, end;
	float delta;
	gettimeofday(&start, NULL);
	my_pthread_t t1,t2,t3,t4;
    int *retVal1, *retVal2, *retVal3, *retVal4;
    //Create threads
    my_pthread_create(&t1, NULL, &threadFunc1,NULL);
    my_pthread_create(&t2, NULL, &threadFunc2,NULL);
    my_pthread_create(&t3, NULL, &threadFunc3,NULL);
    my_pthread_create(&t4, NULL, &threadFunc4,NULL);

    my_pthread_join(t1, &retVal1);
    my_pthread_join(t2, &retVal2);
    my_pthread_join(t3, &retVal3);
    my_pthread_join(t4, &retVal4);
    // printf("Retvals - %d %d %d %d\n",*retVal1,*retVal2,*retVal3, *retVal4);
    gettimeofday(&end, NULL);
    delta = (((end.tv_sec  - start.tv_sec)*1000) + ((end.tv_usec - start.tv_usec)*0.001));
    printf("Execution time in Milliseconds: %f\n",delta);    
    printf("Ending main\n");
    return 0;
}
