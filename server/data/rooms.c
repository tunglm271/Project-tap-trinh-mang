#include "rooms.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Room rooms[MAX_ROOMS];
int num_rooms = 0;

int add_room(const char *name, const char *creator) {
    if (num_rooms >= MAX_ROOMS) {
        printf("Cannot add more rooms. Maximum limit reached.\n");
        return -1;
    }

    Room *new_room = &rooms[num_rooms];
    new_room->id = num_rooms + 1;
    strncpy(new_room->name, name, MAX_NAME_LEN - 1);
    new_room->name[MAX_NAME_LEN - 1] = '\0';

    strncpy(new_room->creator, creator, MAX_NAME_LEN - 1);
    new_room->creator[MAX_NAME_LEN - 1] = '\0';

    new_room->num_users = 1;
    strncpy(new_room->users[0], creator, MAX_NAME_LEN - 1);
    new_room->users[0][MAX_NAME_LEN - 1] = '\0';

    num_rooms++;
    return new_room->id;
}

void add_user_to_room(int room_id, const char *username) {
    for (int i = 0; i < num_rooms; i++) {
        if (rooms[i].id == room_id) {
            if (rooms[i].num_users >= MAX_USERS) {
                printf("Room %d is full. Cannot add user %s.\n", room_id, username);
                return;
            }

            strncpy(rooms[i].users[rooms[i].num_users], username, MAX_NAME_LEN - 1);
            rooms[i].users[rooms[i].num_users][MAX_NAME_LEN - 1] = '\0';

            rooms[i].num_users++;
            return;
        }
    }
}

void remove_user_from_room(int room_id, const char *username) {
    for (int i = 0; i < num_rooms; i++) {
        if (rooms[i].id == room_id) {
            int user_index = -1;
            for (int j = 0; j < rooms[i].num_users; j++) {
                if (strcmp(rooms[i].users[j], username) == 0) {
                    user_index = j;
                    break;
                }
            }

            if (user_index == -1) {
                printf("User %s not found in room %d.\n", username, room_id);
                return;
            }

            if (strcmp(rooms[i].creator, username) == 0) {
                if (rooms[i].num_users > 1) {
                    strcpy(rooms[i].creator, rooms[i].users[1]);
                    printf("Room creator changed to %s.\n", rooms[i].creator);
                } else {
                    printf("Room %d had no other players, deleting room.\n", room_id);
                    remove_room(room_id);
                    return;
                }
            }

            for (int j = user_index; j < rooms[i].num_users - 1; j++) {
                strncpy(rooms[i].users[j], rooms[i].users[j + 1], MAX_NAME_LEN - 1);
                rooms[i].users[j][MAX_NAME_LEN - 1] = '\0';
            }

            rooms[i].num_users--;

            return;
        }
    }

}

void set_room_playing(int room_id) {
     for (int i = 0; i < num_rooms; i++) {
          if (rooms[i].id == room_id) {
              rooms[i].playing = 1;
          }
     }
}

void print_rooms() {
    for (int i = 0; i < num_rooms; i++) {
        printf("Room ID: %d\n", rooms[i].id);
        printf("Room Name: %s\n", rooms[i].name);
        printf("Creator: %s\n", rooms[i].creator);
        printf("Number of Users: %d\n", rooms[i].num_users);
        printf("Users: ");
        for (int j = 0; j < rooms[i].num_users; j++) {
            printf("%s ", rooms[i].users[j]);
        }
        printf("\n\n");
    }
}

char** boardcast_users_first_question_in_rooms(int *num_users_out, const char *username) {
    char *user_list = (char*)malloc(MAX_USERS * sizeof(char*)); 
    int user_count = 0;

    for (int i = 0; i < num_rooms; i++) {
            for (int j = 0; j < rooms[i].num_users; j++) {
                if(strcmp(username, rooms[i].users[j])) {
                    for(int k = 0; k < 10; k++) {
                        int check = strcmp(username, rooms[i].users[k]);
                        if (check != 0) {
                             user_list[user_count] = (char*)malloc(MAX_NAME_LEN * sizeof(char));
                             strncpy(user_list[user_count], rooms[i].users[k], MAX_NAME_LEN);
                             user_count++;
                        }
                    }
                }
                
                
            }
            break;  
    }

    *num_users_out = user_count;
    return user_list;
}

char** boardcast_users_in_rooms(int room_id, int *num_users_out) {
    char **user_list = (char**)malloc(MAX_USERS * sizeof(char*)); 
    int user_count = 0;

    for (int i = 0; i < num_rooms; i++) {
        if (rooms[i].id == room_id) {
            for (int j = 0; j < rooms[i].num_users; j++) {
                int check = strcmp(rooms[i].creator, rooms[i].users[j]);
                if (check != 0) {
                    user_list[user_count] = (char*)malloc(MAX_NAME_LEN * sizeof(char));
                    strncpy(user_list[user_count], rooms[i].users[j], MAX_NAME_LEN);
                    user_count++;
                }
            }
            break;  
        }
    }

    *num_users_out = user_count;
    return user_list;
}

void remove_room(int room_id) {
    int room_index = -1;

    for (int i = 0; i < num_rooms; i++) {
        if (rooms[i].id == room_id) {
            room_index = i;
            break;
        }
    }

    if (room_index == -1) {
        printf("Room with ID %d not found.\n", room_id);
        return;
    }

    for (int i = room_index; i < num_rooms - 1; i++) {
        rooms[i] = rooms[i + 1];
    }

    num_rooms--;

    printf("Room with ID %d removed successfully.\n", room_id);
}
