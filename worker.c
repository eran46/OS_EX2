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

void increment(int counter_file_num) {
    char filename[12];
    snprintf(filename, sizeof(filename), "count%02d.txt", counter_file_num);
    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open counter file"); // ???
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    long long value;
    fscanf(file, "%lld", &value);
    rewind(file);
    fprintf(file, "%lld", value + 1);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
}

void decrement(int counter_file_num) {
    char filename[12];
    snprintf(filename, sizeof(filename), "count%02d.txt", counter_file_num);
    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open counter file"); // ???
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    long long value;
    fscanf(file, "%lld", &value);
    rewind(file); // ??
    fprintf(file, "%lld", value - 1);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);
}

void logfile_out(FILE* logfile, Node* task_node, struct timeval start_time) {
    long long time_ms_end = elapsed_time_ms(start_time);
    fprintf(logfile, "TIME %lld: END job %s\n", time_ms_end, task_node->job_line);
    fflush(logfile);
}

// function to trim leading and trailing spaces
void trim_spaces(char* line) {
	// Remove leading spaces
	int start = 0;
	while (isspace(line[start])) {
	    start++;
	}

	// Move the rest of the string to the front
	memmove(line, line + start, strlen(line) - start + 1);  // Move characters forward

	// Remove trailing spaces
	line[strcspn(line, "\n")] = 0;
	while (isspace(line[strlen(line) - 1])) {
	    line[strlen(line) - 1] = 0;  // Trim trailing spaces
	}
    
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
    	Node* task_node = dequeue(queue); // dequeue has cond locking on threads, waits
    	
    	// task_node = NULL only when thread was asleep and wokeup to empty queue
        if (task_node == NULL) {
	    break;
        }
        
        print_general("thread that just took a job from the queue");
    	printf("%d\n",thread_id);
	
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
	printf("%s\n",task_node->job_line);
	strcpy(cpy_line, task_node->job_line);
	
        if (log_enabled == 1) {
            long long time_ms_start = elapsed_time_ms(program_start_time);
            fprintf(logfile, "TIME %lld: START job %s\n", time_ms_start, task_node->job_line);
            fflush(logfile);
        }
        char* cpy_line_ptr = cpy_line + 6; // past "worker"
        
        
        char *saveptr;
        // char* command = strtok(cpy_line,";"); // strtok is not Thread Safe !!
        char* command = strtok_r(cpy_line_ptr,";",&saveptr);
        while(command != NULL){
        	int cmd_len = strlen(command); // length before trimming command
        	trim_spaces(command);
        	printf("%s\n", command);
        	fflush(stdout);
		if (strncmp(command, "msleep", 6) == 0) {
		    long ms;
		    sscanf(command, "msleep %ld", &ms);
		    
		    print_general("thread sleeping");
		    printf("%ld milliseconds\n", ms);
		    msleep(ms);
		    print_general("thread finished sleeping");

		} else if (strncmp(command, "increment", 9) == 0) {
		    char counter_file_num[3];
		    sscanf(command, "increment %s", counter_file_num);
		    increment(str_to_int(counter_file_num));

		} else if (strncmp(command, "decrement", 9) == 0) { 
		    char counter_file_num[3];
		    sscanf(command, "decrement %s", counter_file_num);
		    decrement(str_to_int(counter_file_num));
		    
		}else if (strncmp(command, "repeat", 6) == 0) {
	    		int repeat_count;
	    		// attempt to extract the repeat count from the job line
	    		if (sscanf(command, "repeat %d", &repeat_count) != 1 || repeat_count < 1) {
				fprintf(stderr, "invalid repeat command or non-positive repeat count\n");
				free(task_node);
				continue;
	    		}
	    		char* cmd_ptr = command;
	    		char* repeat_commands = cmd_ptr + cmd_len + 1; // points at rest of line
	    		printf("DEBUG1 %s\n", repeat_commands);
	    		for (int i = 0; i < repeat_count; i++) {
	    		    char* repeat_commands_cpy = (char*)malloc(strlen(repeat_commands)+1);
	    		    if (repeat_commands_cpy == NULL) {
			    	print_error("allocate memory for copy of commands to repeat  in worker");
			    }	
	    		    strcpy(repeat_commands_cpy, repeat_commands);
	    		    char* saveptr1;
	    		    char* shit_token = strtok_r(repeat_commands_cpy, ";", &saveptr1);
	    		    printf("shit token1 print %s\n",shit_token);
	    		    while(shit_token != NULL){
	    		    	trim_spaces(shit_token);
	    		    	if (strncmp(shit_token, "increment", 9) == 0) {
	        			char counter_file_num[3];
	        			sscanf(shit_token, "increment %s", counter_file_num);
	        			increment(str_to_int(counter_file_num));
	    			} else if (strncmp(shit_token, "decrement", 9) == 0) {
	        			char counter_file_num[3];
	        			sscanf(shit_token, "decrement %s", counter_file_num);
	        			decrement(str_to_int(counter_file_num));
	    			} else if (strncmp(shit_token, "msleep", 6) == 0) {
	        			int ms;
	        			sscanf(shit_token, "msleep %d", &ms);
	        			msleep(ms);
	    			}
	    			shit_token = strtok_r(NULL, ";", &saveptr1);
	    			printf("shit token2 %s\n", shit_token);
	    		    }
	    		    free(repeat_commands_cpy);
	    		    command = NULL;
	    		    
	    		}
	    		continue; // if command was repeat, no need to continue
    		}
    		else{
    			print_general("command not recognized, ignoring");
    		}
    		command = strtok_r(NULL, ";", &saveptr);
	}
	free(cpy_line);
	
	// finished job
	long long int job_time_elapsed = elapsed_time_ms(job_start_time);
	update_min_max_sum_times(job_time_elapsed);
	
	
	
	print_general("thread finished job:");
    	printf("%d\n",thread_id);
    	active_threads_counter((int)-1);
    	
    	// log
    	if (log_enabled == 1) {
        	logfile_out(logfile, task_node, program_start_time);
	}		
    }

    if (log_enabled == 1) {
        fclose(logfile);
    }
    
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

