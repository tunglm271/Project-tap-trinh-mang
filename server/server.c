#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>

#include "data/data.h"

#define PORT 8080
#define MAX_CLIENTS 2
#define MAX 1024

#define MAX_QUESTIONS 100

#define QUESTION 0x02
#define ANSWER 0x03
#define RESULT 0x04
#define TURN_NOTIFICATION 0x05

char *choice[] = {"A.", "B.", "C.", "D."};

int main() {
    Quiz quizArray[MAX_QUESTIONS]; 
    int count = 0;
    int level = 1;
    int server_fd, new_socket, client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX] = {0};


    loadQuestions("data/quiz_library_easy.txt", quizArray, &count, level);
    srand(time(0));
    int numberQuestion = (rand() % 40) + 1;
    
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
        
        client_count++;
        printf("Client %d connected\n", client_count);
    }
    
    for (int i = 0; i < client_count; i++)
    {   
       char buffer[MAX] = {0};
       snprintf(buffer, MAX, "%s\n", quizArray[numberQuestion].question);
       for (int i = 0; i < 4; i++) {
          strncat(buffer, choice[i], MAX - strlen(buffer) - 1); 
          strncat(buffer, quizArray[numberQuestion].option[i], MAX - strlen(buffer) - 1); 
          strncat(buffer, "\n", MAX - strlen(buffer) - 1);
       }
       ssize_t count = send(client_sockets[i], buffer, MAX, 0);
    }
    
    close(server_fd);
    return 0;
}
