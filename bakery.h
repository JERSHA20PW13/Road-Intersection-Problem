#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MEMBAR __sync_synchronize()
#define THREAD_COUNT 12

volatile int tickets[THREAD_COUNT];
volatile int choosing[THREAD_COUNT];

volatile int resource;

void lock(int thread) {
	choosing[thread] = 1;
	MEMBAR;
	int max = 0;
	for(int i=0; i<THREAD_COUNT; i++) {
		int ticket = tickets[i];
		max = ticket > max ? ticket : max;
	}

	tickets[thread] = max + 1;

	MEMBAR;
	choosing[thread] = 0;
    	MEMBAR;

	for (int other = 0; other < THREAD_COUNT; ++other) {

        	while (choosing[other]) {}
        	MEMBAR;

                while (tickets[other] != 0 && (tickets[other] < tickets[thread] || (tickets[other] == tickets[thread] && other < thread))) {
        }
    }
}

void unlock(int thread)
{
	MEMBAR;
    	tickets[thread] = 0;
}

