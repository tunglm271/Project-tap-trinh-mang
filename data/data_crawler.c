#include<stdio.h>
#define MAX_BUFFER 1024

typedef struct {
    char question[MAX_BUFFER];
    char option[4][MAX_BUFFER];
    int right_answer;
    int level;
} Quiz;
