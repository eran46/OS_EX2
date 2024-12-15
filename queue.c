#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

// linked list
typedef struct Node {
    int task_id;
    char job_line[256];  // Copy of the job line
    struct Node* next;
} Node;

// TaskQueue structure with linked list
typedef struct {
    Node* front;  // Points to the front of the queue
    Node* rear;   // Points to the rear of the queue
    int count;    // Tracks the number of tasks in the queue
    pthread_mutex_t lock;
    pthread_cond_t cond_nonempty;
} TaskQueue;

// Initialize the linked list 
void init_queue(TaskQueue* queue) {
    queue->front = queue->rear = NULL;
    queue->count = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond_nonempty, NULL);
}

// Enqueue a task 
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
    new_node->job_line[sizeof(new_node->job_line) - 1] = '\0';  // Ensure null-termination
    new_node->next = NULL;

    // Add node to the rear of the queue
    if (queue->rear == NULL) {
        queue->front = queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }

    queue->count++;
    pthread_cond_signal(&queue->cond_nonempty);  // Signal that the queue is not empty
    pthread_mutex_unlock(&queue->lock);
}

// Dequeue a task from the linked list 
Node* dequeue(TaskQueue* queue) {
    pthread_mutex_lock(&queue->lock);

    // Wait until the queue has at least one task
    while (queue->count == 0) {
        pthread_cond_wait(&queue->cond_nonempty, &queue->lock);
    }
    // Remove the front node from the queue
    Node* temp = queue->front;
    queue->front = queue->front->next;
    // If the queue becomes empty, set rear to NULL
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    queue->count--;
    pthread_mutex_unlock(&queue->lock);
    return temp;
}

// Delete the linked list queue and free all resources
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

// ThreadArgs structure to pass arguments to worker threads
typedef struct {
    TaskQueue* queue;
    int thread_id;
    struct timeval start_time;  // Start time of the program
} ThreadArgs;

// Calculate elapsed time in milliseconds
long long elapsed_time_ms(struct timeval start_time) {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    long long seconds = current_time.tv_sec - start_time.tv_sec;
    long long microseconds = current_time.tv_usec - start_time.tv_usec;
    return (seconds * 1000LL) + (microseconds / 1000);
}

// Worker thread function
void* worker_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    TaskQueue* queue = args->queue;
    int thread_id = args->thread_id;
    struct timeval start_time = args->start_time;

    char filename[20];
    snprintf(filename, sizeof(filename), "thread%02d.txt", thread_id);
    FILE* logfile = fopen(filename, "w");
    if (logfile == NULL) {
        perror("Failed to open log file");
        pthread_exit(NULL);
    }
    
    
    while (1) {
        Node* task_node = dequeue(queue);  // Get a task from the queue

        // Log the start of the task
        long long time_ms_start = elapsed_time_ms(start_time);
        fprintf(logfile, "TIME %lld: START job %s\n", time_ms_start, task_node->job_line);
        fflush(logfile);

        // Simulate task processing
        sleep(1);  // Simulate processing time

        // Log the end of the task
        long long time_ms_end = elapsed_time_ms(start_time);
        fprintf(logfile, "TIME %lld: END job %s\n", time_ms_end, task_node->job_line);
        fflush(logfile);

        free(task_node);  // Free the task node
    }

    fclose(logfile);
    return NULL;
}

int main() {
    TaskQueue queue;
    init_queue(&queue);

    const int NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];

    // Record the start time of the program
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Create worker threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].queue = &queue;
        thread_args[i].thread_id = i + 1;
        thread_args[i].start_time = start_time;
        pthread_create(&threads[i], NULL, worker_thread, &thread_args[i]);
    }

    // Simulate reading from a command file and adding tasks
    for (int i = 1; i <= 20; i++) {
        char job_line[256];
        snprintf(job_line, sizeof(job_line), "Job line for task %d", i);  // Simulated job line
        enqueue(&queue, i, job_line);
        printf("Added task %d to the queue: %s\n", i, job_line);
        sleep(0.5);  // Simulate delay between task additions
    }


    destroy_queue(&queue);  // Clean up the queue
    return 0;
}

