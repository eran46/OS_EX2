#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dispatcher.h"
#include "utils.h"

void print_error(char* err) {
	printf("Error: %s\n", err);
}

// arguments: cmdfile.txt num_threads num_counters log_enabled
int main(int argc, char *argv[]) {
    
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
    
    // send to dispatcher
    parse_cmdfile(FIle* cmdfile);
    
    //next?
    
    
    // destroy mutex and cond
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    
    return 0;
}


