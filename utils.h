#ifndef UTILS_H
#define UTILS_H

#include "common.h"

/**
 * @brief increment and decrements global variable active_threads (common.c).
 * 
 * mode=1 - increment active_threads,
 * mode=-1 - decrement active_threads,
 * uses mutex for thread safety.
 * 
 * @param arg void* Pointer, it is a precondition inorder to use
 * the function in the creation routine.
 * example-
 * int temp = 1;
 * pthread_create(&thread, NULL, active_threads_counter, (void*)&temp);
 * - creates Thread thread, calls function threads_counter() with arg void* temp
 */
void* active_threads_counter(void* arg);

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

// DEBUG
void print_general(char* text);
// DEBUG
void print_error(char* err);

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

#endif
