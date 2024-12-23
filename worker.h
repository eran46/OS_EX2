#ifndef WORKER_H
#define WORKER_H
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include "queue.h"
#include "common.h"
#include "utils.h"

typedef struct { //structure to pass multiple arguments to each worker thread
    int thread_id;
} ThreadArgs;

typedef struct {
    ThreadArgs* args;
    pthread_t* threads;
} ptr_threads_args;

void increment(const char* filename);
void decrement(const char* filename);
void logfile_out(FILE* logfile, Node* task_node, struct timeval start_time);
void trim_spaces(char* str);
void update_min_max_sum_times(long long int job_time_elapsed);
void* worker_thread(void* arg);
ptr_threads_args* create_worker_threads(int num_threads);
void destroy_threads(pthread_t* threads, int num_threads);

#endif

