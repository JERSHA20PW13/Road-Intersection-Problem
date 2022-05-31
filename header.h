#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <syscall.h>

void errQuit(const char *errMsg) {
        perror(errMsg);
        exit(1);
}

pthread_t createPthread(void *(*routine)(void *), void *argument) {
	pthread_t threadId;
	if(pthread_create(&threadId, NULL, routine, (void *)argument))
		errQuit("PThread creation failed!");
	return threadId;
}

intptr_t joinPthread(pthread_t threadId) {
	void *status;
	if(pthread_join(threadId, &status))
		errQuit("PThread join failed");
	return (intptr_t) status;
}

void exitPthread(intptr_t retVal) {
	pthread_exit((void *)retVal);
}

