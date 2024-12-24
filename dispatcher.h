#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "common.h"
#include "utils.h"
#include "queue.h"
#include "worker.h"
#include <stdio.h>

/**
 * @brief dispatcher waits untill worker queue empty.
 * 
 * dispatcher waits untill worker queue empty.
 * 
 */
void dispatcher_wait();

/**
 * @brief Parses line and execute command.
 * 
 * Parses line and execute command by dispatcher,
 * or enqueue for worker command.
 * 
 * @param line String command.
 */
void parse_line(char *line);

/**
 * @brief Reads the command file line by line.
 * 
 * Sends each line to parse_line() for further parsing and execution.
 * 
 * @param file Pointer to already-opened command file.
 */
void parse_cmdfile(FILE* file);

/**
 * @brief Main dispatcher function
 */
void dispatcher(FILE* file);

#endif
