#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#include "queue.h"
#include "dispatcher.h"
#include "utils.h"
#include "common.h"


TaskQueue* queue;
pthread_mutex_t mutex;
pthread_cond_t cond;
struct timeval program_start_time;

// arguments: cmdfile.txt num_threads num_counters log_enabled
int main(int argc, char *argv[]) {
    // get program start time
    gettimeofday(&program_start_time, NULL);

    // initialize queue
    init_queue(queue);
    
    // get program arguments
    if (argc != CMD_ARGS_NUM + 1) {
        print_error("incorrect number of command line arguments");
        return 1;
    }
    int num_threads = str_to_int(argv[2]); // max 4096
    int num_counters = str_to_int(argv[3]); // max 100
    log_enabled = str_to_int(argv[4]);
    
    // initialize mutex and cond
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    
    // initialize counter files
    init_counter_files(num_counters);
    
    // initialize threads
    ptr_threads_args* save_ptr = create_worker_threads(num_threads);
    print_general("initialized all worker threads");
    
    // dispatcher
    FILE* cmdfile = fopen(argv[1], "r");
    if (cmdfile == NULL) {
        print_error("Error opening cmdfile.txt");
        // action? cleanup before exit?
    }
    print_general("cmdfile opened");
    dispatcher(cmdfile);
    
    print_general("dispatcher finished");
    destroy_threads(save_ptr->threads, num_threads);
    print_general("threads destroyed");
    
    
    // -------------> statistics output
    struct timeval program_end_time; // get program end time
    gettimeofday(&program_end_time, NULL);
    
    print_statistics(get_elapsed_time_in_ms(program_end_time));
    
    
    // -------------> cleanup
    // free dynamic memmory
    free(save_ptr->threads);
    free(save_ptr->args);
    free(save_ptr);
    
    // close all files
    
     // destroy mutex and cond
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    return 0;
}
