#include "queue.h"
#include <sys/time.h>
#include <pthread.h>

typedef struct { //structure to pass multiple arguments to each worker thread
    int thread_id;
} ThreadArgs;

typedef struct {
    ThreadArgs* args;
    pthread_t* threads;
} ptr_threads_args;
