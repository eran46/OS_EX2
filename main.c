#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#

#ifndef CMD_ARGS_NUM
#define CMD_ARGS_NUM 4
#endif
#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 1024
#endif
#ifndef MAX_CMD_LENGTH
#define  MAX_CMD_LENGTH 256
#endif

void print_error(char* err) {
	printf("Error: %s\n", err);
}

int parse_str_to_int(char* num_str) {
	int num = 0;
	int digit_factor = 1;
	int len = strlen(num_str);
	for(int i = len - 1; i >= 0; i--){
		num += (num_str[i] - '0') * digit_factor;
		digit_factor *= 10;
	}
	return num;
}

// arguments: cmdfile.txt num_threads num_counters log_enabled
int main(int argc, char *argv[]) {
    
    if (argc != CMD_ARGS_NUM + 1) {
        print_error("incorrect number of command line arguments");
        return 1;
    }
    int num_threads = parse_str_to_int(argv[2]);
    int num_counters = parse_str_to_int(argv[3]);
    char log_enabled = parse_str_to_int(argv[4]);
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        print_error("Error opening cmdfile.txt");
    }
    
    // send to dispatcher
    
    //next
    
    return 0;
}
