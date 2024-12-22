#include "queue.h"


#define CMD_ARGS_NUM 4
#define MAX_LINE_LENGTH 1024
#define MAX_CMD_LENGTH 256


extern TaskQueue queue; // main.c
extern pthread_mutex_t mutex; // main.c
extern pthread_cond_t cond; // main.c
extern struct timeval program_start_time; // main.c

extern int active_threads; // common.c
extern int jobs_count; // common.c
extern long long int jobs_time_sum; // common.c
extern long long int jobs_time_min; // common.c
extern long long int jobs_time_max; // common.c
extern char log_enabled; // common.c

