#ifndef QUIZ_LIBRARY_H
#define QUIZ_LIBRARY_H

#define MAX_BUFFER 256
#define MAX_QUESTIONS 100

typedef struct {
    char question[MAX_BUFFER];
    char option[4][MAX_BUFFER];
    int right_answer;
    int level; // Difficulty level: 1 for easy, 2 for medium, etc.
} Quiz;

// Function to load questions from a file
void loadQuestions(const char *filename, Quiz quizArray[], int *count, int level);

#endif // QUIZ_LIBRARY_H
