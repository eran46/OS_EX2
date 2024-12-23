#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include "queue.h"

void msleep(int milliseconds) {
    usleep(milliseconds * 1000);  // Convert milliseconds to microseconds as the fun usleep require
}

void increment(const char* filename) {
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open counter file");
        return;
    }

    int value;
    fscanf(file, "%d", &value);
    rewind(file);  // Move back to the beginning of the file
    fprintf(file, "%d\n", value + 1);
    fclose(file);
}

// Decrement the counter in the file
void decrement(const char* filename) {
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open counter file");
        return;
    }

    int value;
    fscanf(file, "%d", &value);
    rewind(file);  // Move back to the beginning of the file
    fprintf(file, "%d\n", value - 1);
    fclose(file);
}

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

        // Process the job line
        if (strncmp(task_node->job_line, "msleep", 6) == 0) {
            int ms;
            sscanf(task_node->job_line, "msleep %d", &ms);
            msleep(ms);
        } else if (strncmp(task_node->job_line, "increment", 9) == 0) {
            char counter_file[256];
            sscanf(task_node->job_line, "increment %s", counter_file);
            increment(counter_file);
        } else if (strncmp(task_node->job_line, "decrement", 9) == 0) {
            char counter_file[256];
            sscanf(task_node->job_line, "decrement %s", counter_file);
            decrement(counter_file);
        } else {
            fprintf(logfile, "Unknown job: %s\n", task_node->job_line);
        }

        // Log the end of the task
        long long time_ms_end = elapsed_time_ms(start_time);
        fprintf(logfile, "TIME %lld: END job %s\n", time_ms_end, task_node->job_line);
        fflush(logfile);

        free(task_node);  // Free the task node
    }

    fclose(logfile);
    return NULL;
}

