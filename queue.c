#include "queue.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>


void init_queue(TaskQueue* queue) {
    queue->front = queue->rear = NULL;
    queue->count = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond_nonempty, NULL);
}


void enqueue(TaskQueue* queue, const char* job_line) {
    pthread_mutex_lock(&queue->lock);
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for new node"); // ???
        pthread_mutex_unlock(&queue->lock);
        return;
    }
    strncpy(new_node->job_line, job_line, sizeof(new_node->job_line) - 1);
    new_node->job_line[sizeof(new_node->job_line) - 1] = '\0';
    new_node->next = NULL;

    if (queue->rear == NULL) {
        queue->front = queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
    queue->count++;
    pthread_cond_signal(&queue->cond_nonempty);
    pthread_mutex_unlock(&queue->lock);
}

Node* dequeue(TaskQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    while (queue->count == 0) { // if the queue is empty
    	if(dispatcher_done_flag == 1){ 
    		// on wakeup if dispatcher done and queue empty return NULL
    		pthread_mutex_unlock(&queue->lock);
    		return NULL;
    	}
    	// thread will sleep until signal or broadcast  
    	printf("a thread is waiting for the queue to fill");
        pthread_cond_wait(&queue->cond_nonempty, &queue->lock);
    }
    Node* temp = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    queue->count--;
    pthread_mutex_unlock(&queue->lock);
    return temp;
}

void destroy_queue(TaskQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    Node* current = queue->front;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    pthread_mutex_unlock(&queue->lock);
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->cond_nonempty);
}

long long int elapsed_time_ms(struct timeval start_time) { //time the task in the queue

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    long long seconds = current_time.tv_sec - start_time.tv_sec;
    long long microseconds = current_time.tv_usec - start_time.tv_usec;
    return (seconds * 1000LL) + (microseconds / 1000);
}
