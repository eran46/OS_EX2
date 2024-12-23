#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include "worker.h"
#include <ctype.h>


pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex to protect file access

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

// function to trim leading and trailing spaces
void trim_spaces(char* str) {
    char* end;

    // trim leading spaces
    while (isspace((unsigned char)*str)) str++;

    // trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    // write new null terminator
    *(end + 1) = '\0';
}


void update_min_max_sum_times(long long int job_time_elapsed){
    if(job_time_elapsed >= jobs_time_max){
    	jobs_time_max = job_time_elapsed;
    }
    if(job_time_elapsed <= jobs_time_min){
    	jobs_time_min = job_time_elapsed;
    }
    jobs_time_sum += job_time_elapsed;
}

// arg = args[i]
void* worker_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int thread_id = args->thread_id;
    
    print_general("thread that started working:");
    printf("%d\n",thread_id);
    
    
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
    
    
    while (1) {
    	// first dequeue
    	Node* task_node = dequeue(&queue); // dequeue has cond locking on threads, waits
    	
    	print_general("thread that just took a job from the queue");
    	printf("%d\n",thread_id);
    	
    	// task_node = NULL only when thread was asleep and wokeup to empty queue
        if (task_node == NULL) {
	    break;
        }
	
	// makes a warning
        //if (task_node->job_line == NULL || strlen(task_node->job_line) == 0) { // bad line in queue
        //    free(task_node);
        //    continue;
        // }
        
        // ----------> continue if thread has "good" job
	struct timeval job_start_time;
	gettimeofday(&job_start_time, NULL);
        jobs_count ++;
	active_threads_counter((int)1);
	
	// hard copy string for token and trailing/leading space removal
	char* cpy_line = malloc(strlen(task_node->job_line) + 1); 
	if (cpy_line == NULL) {
	    print_error("allocate memory for copy of string in worker");
	}
	
        if (log_enabled == 1) {
            long long time_ms_start = elapsed_time_ms(program_start_time);
            fprintf(logfile, "TIME %lld: START job %s\n", time_ms_start, task_node->job_line);
            fflush(logfile);
        }
        // task_node->job_line
        char* command = strtok(cpy_line,";");
        while(command != NULL){
        	trim_spaces(command);
        	printf("%s\n", command);
		if (strncmp(command, "msleep", 6) == 0) {
		    long ms;
		    sscanf(command, "msleep %ld", &ms);
		    
		    print_general("thread sleeping");
		    printf("%ld seconds\n", ms);
		    msleep(ms);
		    print_general("thread finished sleeping");

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
				fprintf(stderr, "invalid repeat command or non-positive repeat count\n"); // ???
				free(task_node);
				continue;
	    		}
	    		char* repeat_commands = strchr(command, ';');
	    		repeat_commands++; // start of commands to repeat
	    		for (int i = 0; i < repeat_count; i++) {
	    		    char* repeat_commands_cpy = (char*)malloc(strlen(repeat_commands)+1);
	    		    if (repeat_commands_cpy == NULL) {
			    	print_error("allocate memory for copy of commands to repeat  in worker");
			    }	
	    		    strcpy(repeat_commands_cpy, repeat_commands);
	    		    char* shit_token = strtok(repeat_commands_cpy, ";");
	    		    while(shit_token != NULL){
	    		    	if (strncmp(shit_token, "increment", 9) == 0) {
	        			char counter_file[256];
	        			sscanf(shit_token, "increment %s", counter_file);
	        			increment(counter_file);
	    			} else if (strncmp(shit_token, "decrement", 9) == 0) {
	        			char counter_file[256];
	        			sscanf(shit_token, "decrement %s", counter_file);
	        			decrement(counter_file);
	    			} else if (strncmp(shit_token, "msleep", 6) == 0) {
	        			int ms;
	        			sscanf(shit_token, "msleep %d", &ms);
	        			msleep(ms);
	    			}
	    			shit_token = strtok(NULL, ";");
	    		    }
	    		    free(repeat_commands_cpy);
	    		    command = NULL;
	    		    
	    		}
	    		continue; // if command was repeat, no need to continue
    		}
    		command = strtok(NULL, ";");
	}
	free(cpy_line);
	
	// finished job
	long long int job_time_elapsed = elapsed_time_ms(job_start_time);
	update_min_max_sum_times(job_time_elapsed);
	
	if (log_enabled == 1) {
        	logfile_out(logfile, task_node, program_start_time);
	}  		
        free(task_node);
        
        task_node = dequeue(&queue); 
    }

    if (log_enabled == 1) {
        fclose(logfile);
    }
    active_threads_counter((int)-1);
    return NULL; // return NULL finishes the thread
}

ptr_threads_args* create_worker_threads(int num_threads) {
    ptr_threads_args* ptr_save = (ptr_threads_args*)malloc(sizeof(ptr_threads_args));
    if(ptr_save == NULL){
    	print_error("allocating ptr_save");
    }
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t)*num_threads); // dynamic array of threads
    if(threads == NULL){
    	print_error("allocating threads array");
    }
    ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs)); // holds threads queue, hw2 start time and 
    if(args == NULL){
    	print_error("allocating threads arguments array");
    }
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
void destroy_threads(pthread_t* threads, int num_threads){
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL); // wait for 
    }
}

