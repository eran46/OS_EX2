#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include "queue.h"
#include "common.h"
#include "utils.h"
#define NUM_WORKERS num_threads


pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex to protect file access

void msleep(int milliseconds) {
    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}

void increment(const char* filename) {
    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open counter file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    int value;
    fscanf(file, "%d", &value);
    rewind(file);
    fprintf(file, "%d\n", value + 1);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
}

void decrement(const char* filename) {
    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open counter file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    int value;
    fscanf(file, "%d", &value);
    rewind(file);
    fprintf(file, "%d\n", value - 1);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
}

void logfile_out(FILE* logfile, Node* task_node, struct timeval start_time) {
    long long time_ms_end = elapsed_time_ms(start_time);
    fprintf(logfile, "TIME %lld: END job %s\n", time_ms_end, task_node->job_line);
    fflush(logfile);
}


void* worker_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    TaskQueue* queue = args->queue;
    int thread_id = args->thread_id;
    FILE* logfile = NULL;
    if (log_enable == 1) {
        char filename[20];
        snprintf(filename, sizeof(filename), "thread%02d.txt", thread_id);
        logfile = fopen(filename, "w");
        if (logfile == NULL) {
            perror("Failed to open log file");
            pthread_exit(NULL);
        }
    }

    while (1) {
        Node* task_node = dequeue(queue);
        struct timeval start_time = args->start_time;
        jobs_count ++;
   
        if (task_node == NULL) {
            continue; // exit loop if no tasks are available
        }

        if (task_node->job_line == NULL || strlen(task_node->job_line) == 0) {
            free(task_node);
            continue;
        }

        if (log_enable == 1) {
            long long time_ms_start = elapsed_time_ms(start_time);
            fprintf(logfile, "TIME %lld: START job %s\n", time_ms_start, task_node->job_line);
            fflush(logfile);
        }


        if (strncmp(task_node->job_line, "msleep", 6) == 0) {
            int ms;
            sscanf(task_node->job_line, "msleep %d", &ms);
            msleep(ms);
            if (log_enable == 1) logfile_out(logfile, task_node, start_time);


        } else if (strncmp(task_node->job_line, "increment", 9) == 0) {
            char counter_file[256];
            sscanf(task_node->job_line, "increment %s", counter_file);
            increment(counter_file);
            if (log_enable == 1) logfile_out(logfile, task_node, start_time);


        } else if (strncmp(task_node->job_line, "decrement", 9) == 0) {
            char counter_file[256];
            sscanf(task_node->job_line, "decrement %s", counter_file);
            decrement(counter_file);
            if (log_enable == 1) logfile_out(logfile, task_node, start_time);

	}else if (strncmp(task_node->job_line, "repeat", 6) == 0) {
    		int repeat_count;
    		// attempt to extract the repeat count from the job line
    		if (sscanf(task_node->job_line, "repeat %d", &repeat_count) != 1 || repeat_count < 1) {
        		fprintf(stderr, "invalid repeat command or non-positive repeat count\n");
        		free(task_node);
        		continue;
    		}

    		// find the job part of the repeat line (after the first space)
    		char* job = strchr(task_node->job_line, ';');
    		if (job) {
        		job++; // move past the semicolon to the job description
        		// repeat the task based on the repeat count
        		for (int i = 0; i < repeat_count; i++) {
            			if (strncmp(job, "increment", 9) == 0) {
                			char counter_file[256];
                			sscanf(job, "increment %s", counter_file);
                			increment(counter_file);
            			} else if (strncmp(job, "decrement", 9) == 0) {
                			char counter_file[256];
                			sscanf(job, "decrement %s", counter_file);
                			decrement(counter_file);
            			} else if (strncmp(job, "msleep", 6) == 0) {
                			int ms;
                			sscanf(job, "msleep %d", &ms);
                			msleep(ms);
            			} 
			}
            		// log the repeated task if logging is enabled
            		if (log_enable == 1) {
                	fprintf(logfile, "TIME %lld: REPEAT job %s\n", elapsed_time_ms(start_time), job);
                	fflush(logfile);
            		}
        		}
    		}
    		// job finished
    		if (
	}
    free(task_node);
    if (log_enable == 1) {
        fclose(logfile);
    }
    return NULL;
}

void create_worker_threads(TaskQueue* queue, struct timeval start_time) {
    pthread_t threads[NUM_WORKERS];
    ThreadArgs args[NUM_WORKERS];
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        args[i].queue = queue;
        args[i].thread_id = i + 1;
        args[i].start_time = start_time;
        if (pthread_create(&threads[i], NULL, worker_thread, &args[i]) != 0) {
            perror("Failed to create worker thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(threads[i], NULL);
    }
}

