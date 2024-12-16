
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

