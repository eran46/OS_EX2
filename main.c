#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "common.h"
#include "dispatcher.h"
#include "utils.h"

// global task queue object
TaskQueue queue;

// arguments: cmdfile.txt num_threads num_counters log_enabled
int main(int argc, char *argv[]) {
    struct timeval start_time; // get program start time
    gettimeofday(&start_time, NULL);

    init_queue(&queue); // gets queue POINTER
    if (argc != CMD_ARGS_NUM + 1) {
        print_error("incorrect number of command line arguments");
        return 1;
    }
    int num_threads = str_to_int(argv[2]);
    int num_counters = str_to_int(argv[3]);
    char log_enabled = str_to_int(argv[4]);
    FILE* cmdfile = fopen(argv[1], "r");
    if (file == NULL) {
        print_error("Error opening cmdfile.txt");
    }
    
    // initialize mutex and cond
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    
    // initialize counter files
    init_counter_files(num_counters);
    
    // send to dispatcher
    parse_cmdfile(FIle* cmdfile);
    
    // wait for all working threads to finish
    dispatcher_wait();
    
    //statistics output
    struct timeval end_time; // get program start time
    gettimeofday(&end_time, NULL);
    print_statistics(end_time-start_time); // is this long long int?
    
    // destroy mutex and cond
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    // cleanup
    // close all files
    
    
    return 0;
}


