#include "utils.h"
#include <stdio.h>
#include <unistd.h>


// mode: -1 decrement, 1 increment
void* active_threads_counter(void* arg) { // void* arg explained in docstring
	pthread_mutex_lock(&mutex);
	switch(&mode) {
	case (1):
		active_threads++;
	case (-1):
		active_threads--;
	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}

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

void init_dispatcher_log() {
	char filename[15];
	sprintf(filename, "dispatcher.txt");
	File* f = fopen(filename, "w");
	if(f == NULL) {
		print_error("failed to open file");
	}
	fclose(f);
}

void print_general(char* text) {
	printf("general print: %s\n",text);
}

void print_error(char* err) {
	printf("Error: %s\n", err);
}

// returns time since program start in ms in long long int
long long int get_elapsed_time_in_ms(struct timeval current) {
    return (current.tv_sec - program_start_time.tv_sec) * 1000LL + (current.tv_usec - program_start_time.tv_usec) / 1000LL;
}

void print_statistics(long long int program_time) {
	File* f = fopen("stats.txt", "w");
	fprintf(f, "total running time: %lld milliseconds\n", program_time);
	fprintf(f , "sum of jobs turnaround time: %lld milliseconds\n", jobs_time_sum);
	fprintf(f , "min job turnaround time: %lld milliseconds\n", jobs_time_min);
	fprintf(f, "average job turnaround time: %f milliseconds\n", (jobs_time_sum/jobs_count));
	fprintf(f, "max job turnaround time: %lld milliseconds\n", jobs_time_max);
	fclose(f);
}


void msleep(long time_ms) {
    usleep(time_ms * 1000);  // Convert milliseconds to microseconds
}

