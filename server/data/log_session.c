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