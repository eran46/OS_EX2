#ifndef UTILS_H
#define UTILS_H
#include "common.h"


void* active_threads_counter(int mode);

/**
 * @brief convert numerical string to integer int.
 *
 * convert numerical string to integer int.
 *
 * @param num_str numeric String.
 * @return numeric string as integer.
 */
int str_to_int(char* num_str);

/**
 * @brief initializes num_counters counter files
 *
 * initializes num_counters counter files named "count%02d.txt",num_counters.
 *
 * @param num_counters Integer number of counter files to initiate.
 */
void init_counter_files(int num_counters);

/**
 * @brief initializes dispatcher dispatcher.txt log file
 */
void init_dispatcher_log();

/**
 * @brief returns elapsed time since the start of the program to current time.
 * @param current timeval current time.
 * @return current - program_start_time in milliseconds.
 */
long long int get_elapsed_time_in_ms(struct timeval current);

/**
 * @brief prints the required statistics.
 *
 * using global variables defined in common.c, updated in main.c and worker.c
 *
 * @param program_time long long int program elapsed time.
 */
void print_statistics(long long int program_time);

/**
 * @brief sleep for milliseconds ms.
 *
 * implemented using usleep() from standard library <unistd.h>.
 *
 * @param time_ms long sleep time in milliseconds.
 */
void msleep(long time_ms);
void trim_spaces(char* line);

#endif
