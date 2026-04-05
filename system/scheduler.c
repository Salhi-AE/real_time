#define _GNU_SOURCE
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include "../app/engine.h"

#define NS_PER_MS 1000000

static double price = 65000;

void add_ms(struct timespec *t, long ms) {
    t->tv_nsec += ms * NS_PER_MS;
    while (t->tv_nsec >= 1000000000) {
        t->tv_sec++;
        t->tv_nsec -= 1000000000;
    }
}

void* market_thread(void* arg) {
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    while (1) {
        add_ms(&next, 100);

        price += ((rand()%200) - 100) / 10.0;

        process_price(price);

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
}

void start_system() {
    pthread_t t;
    pthread_create(&t, NULL, market_thread, NULL);
    pthread_join(t, NULL);
}