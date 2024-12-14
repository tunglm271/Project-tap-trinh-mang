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
#include "data/rooms.h"
#include "data/log_session.h"

#define PORT 8080
#define MAX_CLIENTS 20
#define MAX 8192

#define MAX_QUESTIONS 100

#define QUESTION 0x02
#define ANSWER 0x03
#define RESULT 0x04
#define TURN_NOTIFICATION 0x05

typedef struct {
    int id;
    char name[MAX];
} User;

User user_name[MAX];

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
    int max_sd;
    int server_fd, new_socket, client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX] = {0};
    int check_render_room[MAX] = {0};
    

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
    
    fd_set readfds;
    struct timeval timeout;
    int questionEasy[MAX_QUESTIONS];
    int questionMedium[MAX_QUESTIONS];
    int questionHard[MAX_QUESTIONS];
    
    int indexEasy=0;
    int indexMedium=0;
    int indexHard=0;
    
    int numberQuestion;
    int number=0;
    int previousNumber;
    
    while(indexEasy < 10) {
        do {
           numberQuestion = (rand() % 40) + 1;
         } while(isNumberInArray(questionEasy, 10, numberQuestion));
             
        questionEasy[indexEasy] = numberQuestion;
        indexEasy++;
    }
    
    while(indexMedium < 20) {
        do {
           numberQuestion = (rand() % 40) + 1;
         } while(isNumberInArray(questionMedium, 10, numberQuestion));
             
        questionMedium[indexMedium] = numberQuestion;
        indexMedium++;
    }
    
    while(indexHard < 20) {
        do {
           numberQuestion = (rand() % 40) + 1;
         } while(isNumberInArray(questionHard, 10, numberQuestion));
             
        questionHard[indexHard] = numberQuestion;
        indexHard++;
    }
    
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];

            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            if (sd > max_sd) {
                max_sd = sd;
            }
        }
        
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            printf("select error");
            break;
        }
        
           if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                } 
            }
            
           printf("Client %d connected\n", new_socket);
          
          }
        
          for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                char username[50], password[50];
                memset(buffer, 0, MAX);
                if (recv(sd, buffer, MAX, 0) > 0)  {
                    if(buffer[0] == 0x01) {
                    sscanf(buffer+1, "username:%[^;];\npassword:%s\n", username, password);
                    if(login("data/user.txt", username, password) == 1) {
                       memset(buffer, 0, MAX);
                       
                       buffer[0] = 0x02;
                       snprintf(buffer+1, MAX, "%s;%d\n", username, sd);
                       send(client_sockets[i], buffer, MAX, 0);
                       user_name[client_sockets[i]].id = client_sockets[i];
                       strcpy(user_name[client_sockets[i]].name, username);
                        memset(buffer, 0, MAX);
                        buffer[0] = 0x14;
                        memcpy(buffer + 1, &num_rooms, sizeof(int));
                        memcpy(buffer + 1 + sizeof(int), rooms, sizeof(rooms));
                        send(client_sockets[i], buffer, MAX, 0);
                        log_user_session("data/session_log.txt",username, "login");
                            
                    } else { 
                       memset(buffer, 0, MAX); 
                       buffer[0] = 0x03;
                       send(client_sockets[i], buffer, MAX, 0);
                      }
                    }
                 else if (buffer[0] == 0x04) {
                       sscanf(buffer+1, "username:%[^;];\npassword:%s\n", username, password);
                       if(addUserToFile("data/user.txt", username, password, "anh@gmail.com") == 1) {
                       memset(buffer, 0, MAX);
                       buffer[0] = 0x05;
                       send(client_sockets[i], buffer, MAX, 0);
                       log_user_session("data/session_log.txt",username, "register");
                 } else {
                       memset(buffer, 0, MAX);
                       buffer[0] = 0x06;
                       send(client_sockets[i], buffer, MAX, 0);
                       }
                 }
                 else if (buffer[0] == 0x07) {
                   if(number <= 5) {
                     memset(buffer, 0, MAX);
                     snprintf(buffer, MAX, "%s\n", quizArrayEasy[questionEasy[number]].question);
                     for (int i = 0; i < 4; i++) {
                       strncat(buffer, choice[i], MAX - strlen(buffer) - 1);
                       strncat(buffer, quizArrayEasy[questionEasy[number]].option[i], MAX - strlen(buffer) - 1); 
                       strncat(buffer, "\n", MAX - strlen(buffer) - 1);
                    }
                   }
                   
                   if(number > 5 && number <= 10) {
                     memset(buffer, 0, MAX);
                     snprintf(buffer, MAX, "%s\n", quizArrayMedium[questionMedium[number]].question);
                     for (int i = 0; i < 4; i++) {
                       strncat(buffer, choice[i], MAX - strlen(buffer) - 1);
                       strncat(buffer, quizArrayMedium[questionMedium[number]].option[i], MAX - strlen(buffer) - 1); 
                       strncat(buffer, "\n", MAX - strlen(buffer) - 1);
                    }
                   }
                   
                   if(number > 10) {
                     memset(buffer, 0, MAX);
                     snprintf(buffer, MAX, "%s\n", quizArrayHard[questionHard[number]].question);
                     for (int i = 0; i < 4; i++) {
                       strncat(buffer, choice[i], MAX - strlen(buffer) - 1);
                       strncat(buffer, quizArrayHard[questionHard[number]].option[i], MAX - strlen(buffer) - 1); 
                       strncat(buffer, "\n", MAX - strlen(buffer) - 1);
                    }
                   }
                     
                    send(client_sockets[i], buffer, MAX, 0);
                    previousNumber = questionEasy[number];
                 }
                 else if (buffer[0] == 0x08) {
                    if(number <= 5) {
                         if(atoi(buffer+1) == quizArrayEasy[previousNumber].right_answer - 1) {
                         memset(buffer, 0, MAX);
                         buffer[0] = 0x09;
                         send(client_sockets[i], buffer, MAX, 0);
                    }
                    else {
                         memset(buffer, 0, MAX);
                         buffer[0] = 0x10;
                         send(client_sockets[i], buffer, MAX, 0);
                         }   
                    }
                    
                    else if (number > 5 && number <= 10) {
                    if(atoi(buffer+1) == quizArrayMedium[previousNumber].right_answer - 1) {
                     memset(buffer, 0, MAX);
                     buffer[0] = 0x09;
                     send(client_sockets[i], buffer, MAX, 0);
                    }
                    else { 
                      memset(buffer, 0, MAX);
                      buffer[0] = 0x10;
                      send(client_sockets[i], buffer, MAX, 0);
                    }
                    }
                    
                    else if (number > 10) {
                    if(atoi(buffer+1) == quizArrayHard[previousNumber].right_answer - 1) {
                     memset(buffer, 0, MAX);
                     buffer[0] = 0x09;
                     send(client_sockets[i], buffer, MAX, 0);
                    }
                    else { 
                      memset(buffer, 0, MAX);
                      buffer[0] = 0x10;
                      send(client_sockets[i], buffer, MAX, 0);
                    }  
                 } 
                 
                     number++;
                 }    
                 else if(buffer[0] == 0x13) {
                   char room_name[MAX-1];
                   sscanf(buffer + 1, "%[^\n]", room_name);
                   int roomId = add_room(room_name, user_name[client_sockets[i]].name);
                   log_user_session("data/session_log.txt",user_name[client_sockets[i]].name, "create room");
                   check_render_room[i] = 0;

                   memset(buffer, 0, MAX);
                   sprintf(buffer, "%d", roomId);
                   send(client_sockets[i], buffer, MAX, 0);

                   memset(buffer, 0, MAX);
                   buffer[0] = 0x14;
                   memcpy(buffer + 1, &num_rooms, sizeof(int));
                   memcpy(buffer + 1 + sizeof(int), rooms, sizeof(rooms));
                   for(int j=0; j< MAX_CLIENTS; j++) {
                     if(client_sockets[j] > 0 && check_render_room[j] == 1) {
                        send(client_sockets[j], buffer, MAX, 0);
                     }
                   }
                  }
                 else if (buffer[0] == 0x17) {
                     check_render_room[i] = 1;
                 } 
                 else if (buffer[0] == 0x18) {
                     check_render_room[i] = 0;
                 }
                 else if (buffer[0] == 0x15) {
                     char *token = strtok(buffer + 1, ";");
                     int roomId;

                     if (token != NULL) {

                         roomId = atoi(token);

                         token = strtok(NULL, ";");
                     } else {
                           printf("Invalid format: Missing roomId and user_name.\n");
                     }
                       
                     add_user_to_room(roomId, token);
                 }
                 else if (buffer[0] == 0x19) {
                    int roomID = atoi(buffer+1);
                    char socket_in_room[1024]; 
                    int num_users;
                    char **users_in_room = boardcast_users_in_rooms(1, &num_users);
                    
                    for(int k=4; k < 15; k++) {
                         for(int i=0; i < num_users; i++) {
                             int check = strcmp(user_name[k].name, users_in_room[i]);
                             if(check == 0) {
                                 memset(buffer, 0, MAX);
                                 buffer[0] = 0x16;
                                 send(k, buffer, MAX, 0);
                             };
                         }
                    }
                    
                 }
                } 
            }
         }
    }
   
    close(server_fd);
    return 0;
}