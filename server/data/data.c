#include <stdio.h>
#include <string.h>
#include "data.h"

void loadQuestions(const char *filename, Quiz quizArray[], int *count, int level) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    char line[MAX_BUFFER];
    int questionIndex = 0;
    int optionIndex = 0;

    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) {
            // Empty line indicates the end of a question block
            questionIndex++;
            optionIndex = 0;
            continue;
        }

        if (optionIndex == 0) {
            // First line of a block is the question
            snprintf(quizArray[questionIndex].question, MAX_BUFFER, "%s", line);
        } else {
            // Options
            if (strchr(line, '*')) { // Check if it has '*'
                quizArray[questionIndex].right_answer = optionIndex;
                *strchr(line, '*') = '\0'; // Remove the '*' from the option
            }
            snprintf(quizArray[questionIndex].option[optionIndex - 1], MAX_BUFFER, "%s", line);
        }

        optionIndex++;
    }

    // Assign level to all questions
    for (int i = 0; i <= questionIndex; i++) {
        quizArray[i].level = level;
    }

    *count = questionIndex + 1; // Total questions
    fclose(file);
}
