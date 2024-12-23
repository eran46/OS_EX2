#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include "worker.h"
#include "queue.h"
#include "common.h"
#include "utils.h"


pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex to protect file access

void msleep(int milliseconds) {
    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}

void increment(const char* filename) {
    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open counter file"); // ???
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    int value;
    fscanf(file, "%d", &value);
    rewind(file); // ???
    fprintf(file, "%d\n", value + 1);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
}

void decrement(const char* filename) {
    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open counter file"); // ???
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    int value;
    fscanf(file, "%d", &value);
    rewind(file); // ??
    fprintf(file, "%d\n", value - 1);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
}

void logfile_out(FILE* logfile, Node* task_node, struct timeval start_time) {
    long long time_ms_end = elapsed_time_ms(start_time);
    fprintf(logfile, "TIME %lld: END job %s\n", time_ms_end, task_node->job_line);
    fflush(logfile);
}

// Function to trim leading and trailing spaces
void trim_spaces(char* str) {
    char* end;

    // Trim leading spaces
    while (isspace((unsigned char)*str)) str++;

    // Trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end + 1) = '\0';
}

// arg = args[i]
void* worker_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int thread_id = args->thread_id;
    FILE* logfile = NULL;
    if (log_enabled == 1) {
        char filename[20];
        snprintf(filename, sizeof(filename), "thread%02d.txt", thread_id);
        logfile = fopen(filename, "w");
        if (logfile == NULL) {
            perror("Failed to open log file"); // ???
            pthread_exit(NULL); // ???
        }
    }

    while (task_node != NULL && dispatcher_done_flag == 1) {
        Node* task_node = dequeue(queue);
   
        if (task_node == NULL) {
            continue; // if queue empty, keep checking
        }
	
        if (task_node->job_line == NULL || strlen(task_node->job_line) == 0) {
            free(task_node);
            continue;
        }
        
        // ----------> continue if thread has "good" job
	struct timeval job_start_time;
	gettimeofday(&job_start_time, NULL);
        jobs_count ++;
	active_threads_counter(1);
	
        if (log_enabled == 1) { 
            long long time_ms_start = elapsed_time_ms(program_start_time);
            fprintf(logfile, "TIME %lld: START job %s\n", time_ms_start, task_node->job_line);
            fflush(logfile);
        }
        char* command = strtok(task_node->job_line,";");
        while(command != NULL){
        	trim_spaces(command);
		if (strncmp(command, "msleep", 6) == 0) {
		    int ms;
		    sscanf(command, "msleep %d", &ms);
		    msleep(ms);


		} else if (strncmp(command, "increment", 9) == 0) {
		    char counter_file[256];
		    sscanf(command, "increment %s", counter_file);
		    increment(counter_file);

		} else if (strncmp(command, "decrement", 9) == 0) {
		    char counter_file[256];
		    sscanf(command, "decrement %s", counter_file);
		    decrement(counter_file);
		    

		}else if (strncmp(command, "repeat", 6) == 0) {
	    		int repeat_count;
	    		// attempt to extract the repeat count from the job line
	    		if (sscanf(command, "repeat %d", &repeat_count) != 1 || repeat_count < 1) {
				fprintf(stderr,"invalid repeat command or non-positive repeat count\n");
				continue;
	    		}
	    		
// ...;...;...; repeat 5   ;command1;command2
	    		// find the job part of the repeat line (after the first space)
	    		char* repeat_command = strchr(command, ';');
	    		if (repeat_command) {
				repeat_command++; // move past the semicolon to the job description
				// repeat the task based on the repeat count
				for (int i = 0; i < repeat_count; i++) {
		    			if (strncmp(repeat_command, "increment", 9) == 0) {
		        			char counter_file[256];
		        			sscanf(repeat_command, "increment %s", counter_file);
		        			increment(counter_file);
		    			} else if (strncmp(repeat_command, "decrement", 9) == 0) {
		        			char counter_file[256];
		        			sscanf(repeat_command, "decrement %s", counter_file);
		        			decrement(counter_file);
		    			} else if (strncmp(repeat_command, "msleep", 6) == 0) {
		        			int ms;
		        			sscanf(repeat_command, "msleep %d", &ms);
		        			msleep(ms);
		    			}
				}
	    		}
    		}
    		command = strtok(NULL, ";");
	}
	
	// finished job
	if (log_enabled == 1) {
                	logfile_out(logfile, task_node, program_start_time)
	}
            		
        free(task_node);
    }

    if (log_enabled == 1) {
        fclose(logfile);
    }
    active_threads_counter(-1);
    return NULL; // return NULL finishes the thread
}

// ??? - added argument num_threads
ptr_threads_args* create_worker_threads(int num_threads) {
    ptr_threads_args* ptr_save = (ptr_threads_args*)malloc(sizeof(ptr_threads_args));
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t)*num_threads); // dynamic array of threads
    ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs)); // holds threads queue, hw2 start time and 
    ptr_save->args = args;
    ptr_save->threads = threads;
    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        if (pthread_create(&threads[i], NULL, worker_thread, &args[i]) != 0) {
            perror("Failed to create worker thread"); // ???
            exit(EXIT_FAILURE); // ???
        }
    }
    
    return ptr_save;
    
}
void destroy_threads(pthread_t* threads){
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL); // wait for 
    }
}

