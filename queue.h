#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <sys/time.h>

typedef struct Node {
    char job_line[1024];
    struct Node* next;
} Node;
 
typedef struct {
    Node* front;
    Node* rear;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond_nonempty;
} TaskQueue;    //create queue of threads

void init_queue(TaskQueue* queue);

void enqueue(TaskQueue* queue, const char* job_line);

Node* dequeue(TaskQueue* queue);

void destroy_queue(TaskQueue* queue);

long long int elapsed_time_ms(struct timeval start_time);

#endif
