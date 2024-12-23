#include "queue.h"
#include <sys/time.h>

typedef struct { //structure to pass multiple arguments to each worker thread
    TaskQueue* queue;
    int thread_id;
    struct timeval start_time_job;
    struct timeval end_time_job;
} ThreadArgs;

