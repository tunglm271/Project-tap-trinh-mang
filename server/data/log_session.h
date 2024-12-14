#ifndef LOG_SESSION_H
#define LOG_SESSION_H

#include <stdio.h>
#include <string.h>
#include <time.h>

void log_user_session(const char* filename, const char *user_name, const char *action);
void print_success_to_terminal(const char *user_name, const char *action);
void print_error_to_terminal(const char *user_name, const char *action);
void print_log_to_terminal(const char *user_name, const char *action);
#endif