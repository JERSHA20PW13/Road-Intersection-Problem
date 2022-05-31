#include "header.h"
#include "road.h"

#define MAX_CARS_IN_INTERSECTION (3)

sem_t mutex;
sem_t lock;
enum direction inside = none;

void *printRoad(void *arg);

void intersection() {
	sleep(1);
}

void *carThread(void *arg) {
	struct road *roadPtr = (struct road *)arg;
	pthread_t threadId = pthread_self();		//get thred id

	while(roadPtr->blocks[roadPtr->size - 1].car != threadId)
		usleep(100000);

roadEntry:

	while(inside != roadPtr->dir) {
		if(sem_trywait(&mutex) == 0)
			break;
	}

	inside = roadPtr->dir;

	while(TRUE) {
		if(sem_trywait(&lock) == 0)
			break;
		usleep(1000000);
		if(inside != roadPtr->dir)
			goto roadEntry;
	}

	roadPop(roadPtr);
	intersection();
	sem_post(&lock);

	int lockCount;
	sem_getvalue(&lock, &lockCount);

	if(lockCount == MAX_CARS_IN_INTERSECTION)
		sem_post(&mutex);

	return NULL;
}

int main(int argc, char const *argv[])
{
	const unsigned carsCount = MAX_CARS_IN_INTERSECTION * 3;
	const unsigned n = 10;

	pthread_t cars_we[carsCount];
	pthread_t cars_ns[carsCount];

	struct road road_we;
	struct road road_ns;
	struct road *roads[2] = {&road_we, &road_ns};

	sem_init(&lock, 0, MAX_CARS_IN_INTERSECTION);
	sem_init(&mutex, 0, 1);

	initRoad(&road_we, n, ROAD_WTOE);
	initRoad(&road_ns, n, ROAD_NTOS);

	pthread_t mover_we = createPthread(trafficControl, &road_we);
	pthread_t mover_ns = createPthread(trafficControl, &road_ns);
	pthread_t printer = createPthread(printRoad, &roads);

	for(int i=0; i<carsCount; i++) {
		cars_we[i] = createPthread(carThread, &road_we);
		cars_ns[i] = createPthread(carThread, &road_ns);
		roadPush(&road_we, cars_we[i]);
		roadPush(&road_ns, cars_ns[i]);
	}

	for(int i=0; i<carsCount; i++) {
		joinPthread(cars_we[i]);
		joinPthread(cars_ns[i]);
	}

	pthread_cancel(mover_we);
	pthread_cancel(mover_ns);

	joinPthread(mover_we);
	joinPthread(mover_ns);

	pthread_cancel(printer);
	joinPthread(printer);

	sem_destroy(&lock);
	sem_destroy(&mutex);
	destroyRoad(&road_we);
	destroyRoad(&road_ns);

	return 0;
}

void *printRoad(void *arg) {
    struct road **roadPtr = (struct road **)arg;
    while(TRUE) {
	printf("West to East: %d\n", roadPtr[ROAD_WTOE]->car_count);
	printf("North to South: %d\n", roadPtr[ROAD_NTOS]->car_count);

	for(int i=0; i<roadPtr[ROAD_NTOS]->size; i++) {
		for(int j=0; j<roadPtr[ROAD_WTOE]->size; j++)
			fprintf(stdout, "    ");

		if(roadPtr[ROAD_NTOS]->blocks[i].available)
			fprintf(stdout, "[ ] ");
		else
			fprintf(stdout, "[*] ");
		fprintf(stdout, "\n");
	}

	for (int i = 0; i < roadPtr[ROAD_WTOE]->size; i++)
            if (roadPtr[ROAD_WTOE]->blocks[i].available)
                fprintf(stdout, "[ ] ");
            else
                fprintf(stdout, "[*] ");
        fprintf(stdout, "\n");

	pthread_testcancel();
	//usleep(roadPtr[ROAD_WTOE]->speed);
	usleep(1000000);
        system("clear");
    }

     return NULL;
}
