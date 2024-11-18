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

int findUserByUsername(const char *filename, const char *username) {
    FILE *file = fopen(filename, "r"); // Mở file để đọc
    if (!file) {
        perror("Khong the mo file");
        return 0; // Trả về 0 nếu không mở được file
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Loại bỏ ký tự xuống dòng (nếu có)
        line[strcspn(line, "\n")] = '\0';

        // So sánh từng dòng với username
        if (strcmp(line, username) == 0) {
            fclose(file); // Đóng file trước khi trả kết quả
            return 1;     // Tìm thấy username
        }
    }

    fclose(file); // Đóng file nếu không tìm thấy
    return 0;     // Không tìm thấy username
}

// Hàm thêm người dùng mới
int addUserToFile(const char *filename, const char *username, const char *password, const char *email) {
    // Kiểm tra username đã tồn tại chưa
    if (findUserByUsername(filename, username)) {
        printf("Username '%s' da ton tai.\n", username);
        return 0; // Người dùng đã tồn tại
    }

    FILE *file = fopen(filename, "r+");
    if (!file) {
        perror("Không thể mở file");
        return 0;
    }

    // Đọc ID cao nhất từ đầu file
    int highestID;
    if (fscanf(file, "%d", &highestID) != 1) {
        fclose(file);
        return 0;
    }

    // Tăng ID lên 1
    int newID = highestID + 1;

    // Đưa con trỏ về đầu file để ghi đè ID mới
    rewind(file);
    fprintf(file, "%d\n", newID);

    // Đưa con trỏ về cuối file để ghi thêm người dùng mới
    fseek(file, 0, SEEK_END);
    fprintf(file, "\n%d\n%s\n%s\n%s\n", newID, username, password, email);

    fclose(file);
    return 1; // Thành công
}

// Hàm kiểm tra username và mật khẩu
int login(const char *filename, const char *username, const char *password) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Khong the mo file");
        return 0; // Lỗi mở file
    }

    char line[256];
    int idFound = 0;  // Cờ đánh dấu khi tìm thấy username
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Loại bỏ ký tự xuống dòng
        
        // Nếu username được tìm thấy
        if (idFound && strcmp(line, password) == 0) {
            fclose(file);
            return 1; // Mật khẩu khớp, đăng nhập thành công
        }

        // Kiểm tra username
        if (strcmp(line, username) == 0) {
            idFound = 1; // Đặt cờ tìm thấy username
        } else if (idFound) {
            // Nếu đã tìm thấy username nhưng mật khẩu không khớp
            fclose(file);
            return 0; // Sai mật khẩu
        }
    }

    fclose(file);
    return 0; // Không tìm thấy username
}

// int main() {
//     const char *filename = "user.txt";
//     const char *usernameToFind = "hiew1";

//     const char *username = "hiew6";
//     const char *password = "password123";

//     if (login(filename, username, password)) {
//         printf("Dang nhap thanh cong.\n");
//     } else {
//         printf("Sai ten dang nhap hoac mat khau.\n");
//     }

//     return 0;
// }
