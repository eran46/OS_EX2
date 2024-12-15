

// TODO: action per each command 
void parse_line(const char *line) {
    if (strncmp(line, "dispatcher", 10) == 0) {
        // It's a dispatcher command
        if (strncmp(line + 11, "msleep", 6) == 0) {
            // dispatcher msleep <x>
            long msleep_time = strtol(line + 18, NULL, 10);
            printf("Dispatcher sleep for %ld milliseconds\n", msleep_time);
            usleep(msleep_time * 1000); // Sleep in milliseconds
        } else if (strncmp(line + 11, "wait", 4) == 0) {
            // dispatcher wait
            printf("Dispatcher waiting for worker jobs to complete\n");
            // Implement waiting for workers to finish if needed
        } else {
            printf("Unknown dispatcher command: %s\n", line);
        }
    } else {
        // It's a worker job
        printf("Worker job: %s\n", line);
        // Here, we would push the job to the work queue for the worker threads
    }
}

// Main function to read the command file and parse each line
void parse_cmdfile(char *filename) {
    

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline or spaces
        line[strcspn(line, "\n")] = 0;
        while (isspace(line[strlen(line) - 1])) {
            line[strlen(line) - 1] = 0;  // Trim trailing spaces
        }

        if (strlen(line) == 0) {
            continue;  // Skip empty lines
        }

        parse_line(line);
    }

    fclose(file);
}

