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
    queue = (TaskQueue*)malloc(sizeof(TaskQueue));
    init_queue(queue);
    
    
    // get program arguments
    if (argc != CMD_ARGS_NUM + 1) {
        perror("incorrect number of command line arguments");
    }
    int num_threads = str_to_int(argv[2]); // max 4096
    int num_counters = str_to_int(argv[3]); // max 100
    log_enabled = str_to_int(argv[4]);
    FILE* cmdfile = fopen(argv[1], "r"); // open cmdfile
    if (cmdfile == NULL) {
        perror("Error opening cmdfile.txt");
    }
    
    // initialize mutex and cond
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    
    // initialize counter files
    init_counter_files(num_counters);
    
    // initialize threads
    ptr_threads_args* save_ptr = create_worker_threads(num_threads);
    
    // dispatcher
    dispatcher(cmdfile);

    // join all threads
    destroy_threads(save_ptr->threads, num_threads); 
    
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
    fclose(cmdfile);
    
     // destroy mutex and cond
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    return 0;
}
