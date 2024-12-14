#ifndef LOG_RESULT_H
#define LOG_RESULT_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
typedef struct {
    char username[50];
    char result[50];
} Result;

void log_results(const char *filename, char *username, char *result);
Result* get_user_results(const char *filename, const char *username, int *count);


#endif
