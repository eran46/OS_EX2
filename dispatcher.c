#include <pthread.h>

#include "common.h"
#include "utils.h"
#include "queue.h"


pthread_mutex_t mutex;
pthread_cond_t cond;
int active_threads = 0;

void print_general(char* text) {
	printf("general print: %s\n",text);
}

// mode: -1 decrement, 1 increment
void* active_threads_counter(void* arg, int mode) { // void ptr - must for thread create routine
	pthread_mutex_lock(&mutex);
	switch(mode) {
	case (1):
		active_threads++;
	case (-1):
		active_threads--;
	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}

// function to read the command file and parse each line
void parse_cmdfile(FIle* file) {
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline or spaces
        line[strcspn(line, "\n")] = 0;
        while (isspace(line[strlen(line) - 1])) {
            line[strlen(line) - 1] = 0;  // Trim trailing spaces
        }

        if (strlen(line) == 0) {
            continue;  // Skip empty lines
        }

        parse_line(line);
    }

    fclose(file);
}

void dispatcher_wait() {
	pthread_mutex_lock(&mutex);
	while(active_threads > 0) {
		print_general("waiting for active == 0");
		pthread_cond_wait(&cond, &mutex)
	}
	pthread_mutex_unlock(&mutex);
	print_general("finished waiting");
}

// TODO: action per each command 
void parse_line(const char *line) {
    if (strncmp(line, "dispatcher", 10) == 0) {
        // It's a dispatcher command
        if (strncmp(line + 11, "msleep", 6) == 0) {
            // dispatcher msleep <x>
            token = strtok(line + 18, " ;");
            long sleep_time_ms = str_to_int(token);
            printf("Dispatcher sleep for %ld milliseconds\n", sleep_time_ms);
            msleep(sleep_time_us);
        } else if (strncmp(line + 11, "wait", 4) == 0) {
            // dispatcher wait
            printf("Dispatcher waiting for worker jobs to complete\n");
            dispatcher_wait();
        } else {
            printf("Unknown dispatcher command: %s\n", line);
        }
    } else {
        // It's a worker job
        printf("Worker job: %s\n", line);
        // Here, we would push the job to the work queue for the worker threads
    }
}
