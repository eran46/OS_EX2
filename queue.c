#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

typedef struct Node {
    int task_id;
    char job_line[1024];
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond_nonempty;
} TaskQueue;

void init_queue(TaskQueue* queue) {
    queue->front = queue->rear = NULL;
    queue->count = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond_nonempty, NULL);
}

void enqueue(TaskQueue* queue, int task_id, const char* job_line) {
    pthread_mutex_lock(&queue->lock);
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for new node");
        pthread_mutex_unlock(&queue->lock);
        return;
    }
    new_node->task_id = task_id;
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
    while (queue->count == 0) {
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

long long elapsed_time_ms(struct timeval start_time) {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    long long seconds = current_time.tv_sec - start_time.tv_sec;
    long long microseconds = current_time.tv_usec - start_time.tv_usec;
    return (seconds * 1000LL) + (microseconds / 1000);
}

typedef struct { //structure to pass multiple arguments to each worker thread
    TaskQueue* queue;
    int thread_id;
    struct timeval start_time;
} ThreadArgs;

void* worker_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    TaskQueue* queue = args->queue;
    int thread_id = args->thread_id;
    struct timeval start_time = args->start_time;
    char filename[100];  // we need to check how much char we need
    snprintf(filename, sizeof(filename), "thread%02d.txt", thread_id);
    FILE* logfile = fopen(filename, "w");
    if (logfile == NULL) {
        perror("Failed to open log file");
        pthread_exit(NULL);
    }
    while (1) {
        Node* task_node = dequeue(queue);
        long long time_ms_start = elapsed_time_ms(start_time);
        fprintf(logfile, "TIME %lld: START job %s\n", time_ms_start, task_node->job_line);
        fflush(logfile);

        sleep(1);  // Simulate task processing time

        long long time_ms_end = elapsed_time_ms(start_time);
        fprintf(logfile, "TIME %lld: END job %s\n", time_ms_end, task_node->job_line);
        fflush(logfile);

        free(task_node);
    }

    fclose(logfile);
    return NULL;
}

void start_worker_threads(TaskQueue* queue, int num_threads, struct timeval start_time) {
    pthread_t threads[num_threads];
    ThreadArgs thread_args[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].queue = queue;
        thread_args[i].thread_id = i + 1;
        thread_args[i].start_time = start_time;
        pthread_create(&threads[i], NULL, worker_thread, &thread_args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}
//111
