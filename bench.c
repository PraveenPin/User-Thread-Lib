#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>


pthread_mutex_t mutex, mutex2;

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

int threadFunc1(void*g) {
    printf("Thread 1 is trying to lock the mutex\n");
    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&mutex2);
	int i;
    for(i = 0; i < 2; i++){
        // pthread_mutex_lock(&mutex);
        busyWait(1);
        pthread_mutex_unlock(&mutex);
        //printf("This is the first Thread 1\n");
    }
    pthread_mutex_unlock(&mutex2);
    return 11;
}

void threadFunc(){
    printf("Generic thread function has started\n");
    for(int i = 0; i< 3;i++){
        busyWait(0);
    }
    printf("Generic thread function has completed\n");
}

void *threadFunc2() {
	int i;
    printf("Thread 2 is trying to lock the mutex \n");
    pthread_mutex_lock(&mutex);
    printf("Thread 2 has successfully acquired the lock\n");
    for(i = 0; i < 2 ; i++) {
        busyWait(2);
        //printf("This is the second Thread 2\n");
    }
    printf("Thread 2 is trying to unlock the mutex\n");
    pthread_mutex_unlock(&mutex);
    printf("Thread  2 EXITING!!!!!!!!\n");
}

void *threadFunc3() {
    int i;
    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&mutex2);

    for(i = 0; i < 3 ; i++) {
        busyWait(3);
        //printf("This is the third Thread 3\n");
        pthread_mutex_unlock(&mutex2);
    }
    pthread_mutex_unlock(&mutex);
    printf("Thread  3 is done!\n");
}

void *threadFunc4() {
	int i;
    pthread_mutex_lock(&mutex2);
    for(i = 0; i < 3 ; i++) {
        busyWait(4);
        //printf("This is the fourth Thread 4\n");
    }
    pthread_mutex_unlock(&mutex2);
}

int main(int argc, const char * argv[]) {
	struct timeval start, end;
	float delta;
	gettimeofday(&start, NULL);
	pthread_t t1,t2,t3,t4,t5,t6,t7,t8;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex2, NULL);
    //Create threads
    pthread_create(&t1, NULL, &threadFunc1,NULL);
    pthread_create(&t2, NULL, &threadFunc2,NULL);
    pthread_create(&t3, NULL, &threadFunc3,NULL);
    pthread_create(&t4, NULL, &threadFunc4,NULL);
    pthread_create(&t5, NULL, threadFunc,NULL);
    pthread_create(&t6, NULL, threadFunc,NULL);
    pthread_create(&t7, NULL, threadFunc,NULL);
    pthread_create(&t8, NULL, threadFunc,NULL);
    //Call join on the threads
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    pthread_join(t3,NULL);
    pthread_join(t4,NULL);
    pthread_join(t5,NULL);
    pthread_join(t6,NULL);
    pthread_join(t7,NULL);
    pthread_join(t8,NULL);
    //Destroying the mutex
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex2);
    gettimeofday(&end, NULL);
    delta = (((end.tv_sec  - start.tv_sec)*1000) + ((end.tv_usec - start.tv_usec)*0.001));
    printf("Execution time in Milliseconds: %f\n",delta);
    printf("Ending main!\n");
    return 0;
}

