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
                    } else { 
                       memset(buffer, 0, MAX);
                       buffer[0] = 0x03;
                       send(client_sockets[i], buffer, MAX, 0);
                      }
                    }
                 else if(buffer[0] == 0x13) {
                   char room_name[MAX-1];
                   sscanf(buffer + 1, "%[^\n]", room_name);
                   add_room(room_name, user_name[client_sockets[i]].name);
                   memset(buffer, 0, MAX);
                   buffer[0] = 0x14;
                   memcpy(buffer + 1, rooms, sizeof(rooms));
                   send(sd, buffer, MAX, 0);
                  }
                } 
            }
         }
    }
   
    close(server_fd);
    return 0;
}
