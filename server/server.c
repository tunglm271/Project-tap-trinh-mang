#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdbool.h>

#include "data/data.h"

#define PORT 8080
#define MAX_CLIENTS 20
#define MAX 1024

#define MAX_QUESTIONS 100

#define QUESTION 0x02
#define ANSWER 0x03
#define RESULT 0x04
#define TURN_NOTIFICATION 0x05

char *choice[] = {"A.", "B.", "C.", "D."};

void handle_login_request(int client_socket) {
    char buffer[MAX] = {0};
    recv(client_socket, buffer, MAX, 0);
    // Process login request (this is a placeholder, implement your login logic here)
    printf("Received login request: %s\n", buffer);
    // Send login response (success or failure)
    const char *response = "Login successful\n";
    send(client_socket, response, strlen(response), 0);
}

void handle_register_request(int client_socket) {
    char buffer[MAX] = {0};
    recv(client_socket, buffer, MAX, 0);
    // Process register request (this is a placeholder, implement your register logic here)
    printf("Received register request: %s\n", buffer);
    // Send register response (success or failure)
    const char *response = "Register successful\n";
    send(client_socket, response, strlen(response), 0);
}

bool isNumberInArray(int arr[], int size, int num) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == num) {
            return true;  
        }
    }
    return false;  
}

int main() {
    Quiz quizArrayEasy[MAX_QUESTIONS]; 
    Quiz quizArrayMedium[MAX_QUESTIONS]; 
    Quiz quizArrayHard[MAX_QUESTIONS]; 
    int count = 0;
    int level = 1;
    int server_fd, new_socket, client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX] = {0};
    

    loadQuestions("data/quiz_library_easy.txt", quizArrayEasy, &count, level);
    loadQuestions("data/quiz_library_medium.txt", quizArrayMedium, &count, level);
    loadQuestions("data/quiz_library_hard.txt", quizArrayHard, &count, level);

    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);


    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }


    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening...\n");
    
    int client_count = 0;
    while (client_count < MAX_CLIENTS)
    {   
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        
        client_sockets[client_count] = new_socket;
        
        int previousNumber;
        int number = 1;
        int numberQuestion;
        int questionEasy[MAX_QUESTIONS];
        int questionMedium[MAX_QUESTIONS];
        int questionHard[MAX_QUESTIONS];
        int indexEasy=0;
        int indexMedium=0;
        int indexHard=0;
        
        memset(buffer, 0, MAX);
        char username[50], password[50];
        
        while(recv(client_sockets[client_count], buffer, MAX, 0) > 0) {
       
          srand(time(0));
          
          if(number <= 6) { 
             do {
                numberQuestion = (rand() % 40) + 1;
             } while(isNumberInArray(questionEasy, 10, numberQuestion));
             
             questionEasy[indexEasy] = numberQuestion;
             indexEasy++;
          } else if (number > 6 && number <= 11) {

             do {
                numberQuestion = (rand() % 30) + 1;
             } while(isNumberInArray(questionMedium, 10, numberQuestion));
             
             questionMedium[indexMedium] = numberQuestion;
             indexMedium++;
          } else if (number > 11) {
          
             do {
                numberQuestion = (rand() % 30) + 1;
             } while(isNumberInArray(questionHard, 10, numberQuestion));
             
             questionHard[indexHard] = numberQuestion;
             indexHard++;
          }
          
          if(buffer[0] == 0x01) {
            sscanf(buffer+1, "username:%[^;];\npassword:%s\n", username, password);
               if(login("data/user.txt", username, password) == 1) {
                 memset(buffer, 0, MAX);
                 buffer[0] = 0x02;
                 send(client_sockets[client_count], buffer, MAX, 0);
               } else { 
                 memset(buffer, 0, MAX);
                 buffer[0] = 0x03;
                 send(client_sockets[client_count], buffer, MAX, 0);
               }
            }
           else if (buffer[0] == 0x04) {
              sscanf(buffer+1, "username:%[^;];\npassword:%s\n", username, password);
              if(addUserToFile("data/user.txt", username, password, "anh@gmail.com") == 1) {
                 memset(buffer, 0, MAX);
                 buffer[0] = 0x05;
                 send(client_sockets[client_count], buffer, MAX, 0);
              } else {
                 memset(buffer, 0, MAX);
                 buffer[0] = 0x06;
                 send(client_sockets[client_count], buffer, MAX, 0);
              }
           } else if (buffer[0] == 0x07) {
                if(number <= 6) {
                    printf("%d\n", numberQuestion);
                    memset(buffer, 0, MAX);
                    snprintf(buffer, MAX, "%s\n", quizArrayEasy[numberQuestion].question);
                    for (int i = 0; i < 4; i++) {
                       strncat(buffer, choice[i], MAX - strlen(buffer) - 1); 
                       strncat(buffer, quizArrayEasy[numberQuestion].option[i], MAX - strlen(buffer) - 1); 
                       strncat(buffer, "\n", MAX - strlen(buffer) - 1);
                    } 
                } else if (number > 6 && number <= 11) {
                    printf("%d\n", numberQuestion);
                    memset(buffer, 0, MAX);

                    snprintf(buffer, MAX, "%s\n", quizArrayMedium[numberQuestion].question);
                    for (int i = 0; i < 4; i++) {
                       strncat(buffer, choice[i], MAX - strlen(buffer) - 1); 
                       strncat(buffer, quizArrayMedium[numberQuestion].option[i], MAX - strlen(buffer) - 1); 
                       strncat(buffer, "\n", MAX - strlen(buffer) - 1);
                    } 
                } else if (number > 11) { 
                    printf("%d\n", numberQuestion);
                    memset(buffer, 0, MAX);
                    snprintf(buffer, MAX, "%s\n", quizArrayHard[numberQuestion].question);
                    for (int i = 0; i < 4; i++) {
                       strncat(buffer, choice[i], MAX - strlen(buffer) - 1); 
                       strncat(buffer, quizArrayHard[numberQuestion].option[i], MAX - strlen(buffer) - 1); 
                       strncat(buffer, "\n", MAX - strlen(buffer) - 1);
                    } 
                }
                send(client_sockets[client_count], buffer, MAX, 0);
                previousNumber = numberQuestion;
           } else if (buffer[0] == 0x08) {
                 if(number <= 6) {
                    if(atoi(buffer+1) == quizArrayEasy[previousNumber].right_answer - 1) {
                     memset(buffer, 0, MAX);
                     buffer[0] = 0x09;
                     send(client_sockets[client_count], buffer, MAX, 0);
                    }
                    else { 
                      memset(buffer, 0, MAX);
                      buffer[0] = 0x10;
                      send(client_sockets[client_count], buffer, MAX, 0);
                    }
                 }
                 else if (number > 6 && number <= 11) {
                    if(atoi(buffer+1) == quizArrayMedium[previousNumber].right_answer - 1) {
                     memset(buffer, 0, MAX);
                     buffer[0] = 0x09;
                     send(client_sockets[client_count], buffer, MAX, 0);
                    }
                    else { 
                      memset(buffer, 0, MAX);
                      buffer[0] = 0x10;
                      send(client_sockets[client_count], buffer, MAX, 0);
                    }
                 }
                 else if (number > 11) {
                    if(atoi(buffer+1) == quizArrayHard[previousNumber].right_answer - 1) {
                     memset(buffer, 0, MAX);
                     buffer[0] = 0x09;
                     send(client_sockets[client_count], buffer, MAX, 0);
                    }
                    else { 
                      memset(buffer, 0, MAX);
                      buffer[0] = 0x10;
                      send(client_sockets[client_count], buffer, MAX, 0);
                    }  
                 } 
              number++;   
           } else if (buffer[0] == 0x11) {
               if(number <= 6) {
                   memset(buffer, 0, MAX);
                   buffer[0] = 0x12;
                   snprintf(buffer+1, MAX, "%d\n", quizArrayEasy[previousNumber].right_answer - 1);
                   send(client_sockets[client_count], buffer, MAX, 0);
              } else if(number > 6 && number <= 11) {
                   memset(buffer, 0, MAX);
                   buffer[0] = 0x12;
                   snprintf(buffer+1, MAX, "%d\n", quizArrayMedium[previousNumber].right_answer - 1);
                   send(client_sockets[client_count], buffer, MAX, 0);
              } else if(number > 11) {
                   memset(buffer, 0, MAX);
                   buffer[0] = 0x12;
                   snprintf(buffer+1, MAX, "%d\n", quizArrayHard[previousNumber].right_answer - 1);
                   send(client_sockets[client_count], buffer, MAX, 0);
              }
           }
        }
        
        client_count++;
        printf("Client %d connected\n", client_count);
    }
    
   
    close(server_fd);
    return 0;
}
