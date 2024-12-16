#define CMD_ARGS_NUM 4
#define MAX_LINE_LENGTH 1024
#define  MAX_CMD_LENGTH 256


extern TaskQueue queue;
int jobs_count = 0;
long long int jobs_time_sum = 0;
long long int jobs_time_min = 0;
long long int jobs_time_max = 0;

