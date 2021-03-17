#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>


pthread_mutex_t mutex;

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
/***
 * funThread1 - Function for thread 1. This is the function that is executed when thread 1 is scheduled.
 * @param 	null
 * @return 	null
 */
void thread1() {
	printf("Thread 1 is trying to lock the mutex\n");
    int i;
    for(i = 0; i < 2; i++){
        busyWait(1);
        printf("This is the first Thread 1\n");
    }
}

/***
 * funThread2 - Function for thread 2. This is the function that is executed when thread 2 is scheduled.
 * @param 	null
 * @return 	null
 */
void thread2() {
	int i;
    printf("Thread 2 is trying to lock the mutex \n");
    printf("Thread 2 has successfully acquired the lock\n");
    for(i = 0; i < 2 ; i++) {
        busyWait(2);
        printf("This is the second Thread 2\n");
    }
    printf("Thread 2 is trying to unlock the mutex\n");
    printf("Thread  2 EXITING!!!!!!!!\n");
}

/***
 * funThread3 - Function for thread 3. This is the function that is executed when thread 3 is scheduled.
 * @param 	null
 * @return 	null
 */
void thread3() {
    int i;

    for(i = 0; i < 3 ; i++) {
        busyWait(3);
        printf("This is the third Thread 3\n");
    }

    printf("Thread  3 is done!\n");
}
/***
 * funThread4 - Function for thread 4. This is the function that is executed when thread 4 is scheduled.
 * @param 	null
 * @return 	null
 */
void thread4() {
	int i;
    for(i = 0; i < 3 ; i++) {
        busyWait(4);
        printf("This is the fourth Thread 4\n");
    }
}

int main(int argc, const char * argv[]) {
	struct timeval start, end;
	float delta;
	gettimeofday(&start, NULL);
	thread1();
    thread2();
    thread3();
    thread4();
    gettimeofday(&end, NULL);
    delta = (((end.tv_sec  - start.tv_sec)*1000) + ((end.tv_usec - start.tv_usec)*0.001));
    printf("Execution time in Milliseconds: %f\n",delta);
    printf("Ending main!\n");
    return 0;
}

