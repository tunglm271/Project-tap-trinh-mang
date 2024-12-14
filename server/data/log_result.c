#include "log_result.h"

void log_results(const char *filename, char *username, char *result) {
    FILE *file = fopen(filename, "a+");
    if (file == NULL) {
        perror("Failed to open log file");
        return;
    }

    int stt = 1;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        stt++;
    }

    fprintf(file, "%d %s %s\n", stt, username, result);
    fclose(file);
}


Result* get_user_results(const char *filename, const char *username, int *count) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open log file");
        *count = 0;
        return NULL;
    }

    Result *results = NULL;
    *count = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        char file_username[50];
        char result[50];
        int stt;
        sscanf(buffer, "%d %s %s", &stt, file_username, result);
        if (strcmp(file_username, username) == 0) {
            results = realloc(results, (*count + 1) * sizeof(Result));
            strcpy(results[*count].username, file_username);
            strcpy(results[*count].result, result);
            (*count)++;
        }
    }
    for (int i = 0; i < *count; i++) {
        printf("Result %d: %s %s\n", i + 1, results[i].username, results[i].result);
    }

    fclose(file);
    return results;
}