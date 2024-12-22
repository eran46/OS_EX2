#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

typedef struct Node {
    int task_id;
    char job_line[256];  // Task description
    struct Node* next;   // Pointer to the next node
} Node;


typedef struct {
    Node* front;  // Points to the front of the queue
    Node* rear;   // Points to the rear of the queue
    int count;    // Tracks the number of tasks in the queue
    pthread_mutex_t lock;  // Mutex for synchronizing access
    pthread_cond_t cond_nonempty;  // Condition variable for non-empty queue
} TaskQueue;

void init_queue(TaskQueue* queue);

void enqueue(TaskQueue* queue, int task_id, const char* job_line);

Node* dequeue(TaskQueue* queue);

void destroy_queue(TaskQueue* queue);

#endif 

