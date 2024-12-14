#include "log_session.h"


void log_user_session(const char* filename, const char *user_name, const char *action) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("Failed to open log file");
        return;
    }
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[100];
    strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S", t);
    fprintf(file, "username: %s\ntime: %s\naction: %s\n\n", user_name, time_str, action);
    fclose(file);
}

void print_success_to_terminal(const char *user_name, const char *action) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[100];
    strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S", t);
    printf("\033[1;32musername: %s\t| time: %s\t| action: %s\033[0m\n", user_name, time_str, action);
}

void print_error_to_terminal(const char *user_name, const char *action) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[100];
    strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S", t);
    printf("\033[1;31musername: %s\t| time: %s\t| action: %s\033[0m\n", user_name, time_str, action);
}

void print_log_to_terminal(const char *user_name, const char *action) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[100];
    strftime(time_str, sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S", t);
    printf("username: %s\t| time: %s\t| action: %s\n", user_name, time_str, action);
}