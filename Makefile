CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib


Target: my_pthread.a

my_pthread.a: my_pthread.o queue.o
	$(AR) libmy_pthread.a my_pthread.o queue.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h queue.h
	$(CC) -pthread $(CFLAGS) my_pthread.c queue.c

clean:
	rm -rf testfile *.o *.a
