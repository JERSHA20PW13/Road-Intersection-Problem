#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef unsigned char bool;

#define TRUE (1)
#define FALSE (0)
#define ROAD_DEFAULT_SIZE (10)
#define ROAD_DEAFULT_SPEED (10000)
#define ROAD_WTOE (0)			//west to east
#define ROAD_NTOS (1)			//north to south

#define ACQUIRE(mutex) (pthread_mutex_lock(&mutex))
#define RELEASE(mutex) (pthread_mutex_unlock(&mutex))

enum direction
{
    north_to_south,
    west_to_east,
    none,
};

struct block
{
    pthread_t car;
    bool available;
};

struct road
{
    unsigned size;
    unsigned car_count;
    unsigned speed;

    enum direction dir;
    struct block *blocks;
    pthread_mutex_t lock;
};

void initRoad(struct road *roadPtr, unsigned size, unsigned dir) {
	roadPtr->size = size;
	roadPtr->car_count = 0;
	roadPtr->speed = ROAD_DEAFULT_SPEED;
	roadPtr->dir = dir;
	roadPtr->blocks = (struct block *)malloc(size * sizeof(struct block));
	for(int i=0; i<size; i++) {
		roadPtr->blocks[i].available = TRUE;
	}
	pthread_mutex_init(&roadPtr->lock, 0);
}

void destroyRoad(struct road *roadPtr) {
	free(roadPtr->blocks);
	roadPtr->blocks = NULL;
	pthread_mutex_destroy(&roadPtr->lock);
}

void roadMove(struct road *roadPtr) {
	ACQUIRE(roadPtr->lock);
		unsigned carCount = roadPtr->car_count;
	RELEASE(roadPtr->lock);

	if(carCount == roadPtr->size)		//road is full
		return;

	unsigned i = roadPtr->size - 1;
	while(roadPtr->blocks[i].available == FALSE)
		i--;

	while(i > 0) {
		roadPtr->blocks[i].available =  roadPtr->blocks[i-1].available;
		roadPtr->blocks[i].car = roadPtr->blocks[i - 1].car;
		i--;
	}
	roadPtr->blocks[0].available = TRUE;
}

void roadPush(struct road *roadPtr, pthread_t car) {
	while(roadPtr->blocks[0].available == FALSE)
		usleep(100000);			//suspend execution for microseconds interval

	ACQUIRE(roadPtr->lock);
		roadPtr->car_count++;
	RELEASE(roadPtr->lock);

	roadPtr->blocks[0].available = FALSE;
	roadPtr->blocks[0].car = car;
}

void roadPop(struct road *roadPtr) {
	roadPtr->blocks[roadPtr->size - 1].available = TRUE;

	ACQUIRE(roadPtr->lock);
		roadPtr->car_count--;
	RELEASE(roadPtr->lock);
}

void *trafficControl(void *arg) {
	struct road *roadPtr = (struct road *)arg;
	while(TRUE) {
		roadMove(roadPtr);
		pthread_testcancel();
		usleep(roadPtr->speed * 10);
	}
	return NULL;
}
