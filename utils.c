#include <stdio.h>

#include "common.h"


int str_to_int(char* num_str) {
	int num = 0;
	int digit_factor = 1;
	int len = strlen(num_str);
	for(int i = len - 1; i >= 0; i--){
		num += (num_str[i] - '0') * digit_factor;
		digit_factor *= 10;
	}
	return num;
}

void init_counter_files(int num_counters) {
	for(int i=0; i<num_counters; i++) {
		char filename[12];
		sprintf(filename, "count%02d.txt", i);
		File* f = fopen(filename, "w");
		if(f == NULL) {
			print_error("failed to open file");
		}
		else{
			fprintf(f, "%lld", 0) // NO NEW LINE
			general_print("count file %d initialized\n", i);
		}
		fclose(f);
	}
}

void print_general(char* text) {
	printf("general print: %s\n",text);
}

void print_error(char* err) {
	printf("Error: %s\n", err);
}

void print_statistics(long long int total_time) {
	File* f = fopen("stats.txt", "w");
	fprintf(f, "total running time: %lld milliseconds\n", total_time);
	printf(, "sum of jobs turnaround time: %lld milliseconds\n", jobs_time_sum);
	printf(, "min job turnaround time: %lld milliseconds\n", jobs_time_min);
	printf(, "average job turnaround time: %f milliseconds\n", (jobs_time_sum/jobs_count));
	printf(, "max job turnaround time: %lld milliseconds\n", jobs_time_max);
	fclose(f);
}

