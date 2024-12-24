#include "dispatcher.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void dispatcher_done(){
	pthread_mutex_lock(&queue->lock);
	dispatcher_done_flag = 1;
	pthread_cond_broadcast(&queue->cond_nonempty); // wake up all sleeping threads.
	pthread_mutex_unlock(&queue->lock);
}

// main thread wait until queue is empty
void dispatcher_wait() {
	pthread_mutex_lock(&mutex);
	while(active_threads > 0 || queue->count > 0) {
		pthread_cond_wait(&cond, &mutex);
	}
	pthread_mutex_unlock(&mutex);
}

// parse each line and execute the command
void parse_line(char *line) {
    if (strncmp(line, "dispatcher", 10) == 0) {
        // dispatcher command
        char* token;
        if (strncmp(line + 11, "msleep", 6) == 0) {
            token = strtok(line + 18, " ;");
            long sleep_time_ms = str_to_int(token);
            msleep(sleep_time_ms);
        } else if (strncmp(line + 11, "wait", 4) == 0) {
            dispatcher_wait();
        } else {
            print_error("Unknown dispatcher command");
        }
    }
    else if(strncmp(line, "worker", 6) == 0) {
        // worker job
    	while (isspace((unsigned char)*line)) line++; // trim leading spaces
        enqueue(queue, line);
    }
    else{
    	print_error("Unknown job assignment not to worker or dispatcher");
    }
}

// read the cmdfile.txt and parse each line
void parse_cmdfile(FILE* file) {
    char* line = (char*)malloc(sizeof(char)*MAX_LINE_LENGTH);
    if(line == NULL){
    	print_error("on allocating cmdfile line string");
    }
    while (fgets(line, MAX_LINE_LENGTH, file)) {

    	line[strcspn(line, "\n")] = 0; // Remove trailing newline
    	if(log_enabled == 1){
    	    FILE* dispatcher_log = fopen("dispatcher.txt", "a");
    	    if(dispatcher_log == NULL){
    	    	print_error("dispatcher log failed to open");
    	    }
    	    struct timeval current_time;
    	    gettimeofday(&current_time, NULL);
    	    long long int elapsed_time = get_elapsed_time_in_ms(current_time);
    	    fprintf(dispatcher_log, "TIME %lld: read cmd line: %s\n", elapsed_time, line);
    	    fclose(dispatcher_log);
    	}
    	
    	trim_spaces(line); // trim trailing or leading spaces

	// Skip empty lines
        if (strlen(line) == 0) { 
            continue;
        }
        
        parse_line(line);
    }
    
    dispatcher_done_flag = 1; // marks dispatcher is done reading cmdfile
}

void dispatcher(FILE* file){
    // open dispatcher log
    if(log_enabled == 1){
        init_dispatcher_log();
    }
    
    // parse command file
    parse_cmdfile(file);
    dispatcher_done(); // update dispatcher_done_flag=1 and wake up all sleeping threads
    dispatcher_wait(); // wait for all active threads to finish
}

