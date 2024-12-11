#ifndef ROOMS_H
#define ROOMS_H

#include <stdio.h>
#include <string.h>

#define MAX_USERS 10
#define MAX_NAME_LEN 50
#define MAX_ROOMS 10

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char creator[MAX_NAME_LEN];
    int num_users;
    char users[MAX_USERS][MAX_NAME_LEN];
} Room;

extern Room rooms[MAX_ROOMS];
extern int num_rooms;

void add_room(const char *name, const char *creator);
void add_user_to_room(int room_id, const char *username);
void remove_user_from_room(int room_id, const char *username);
void remove_room(int room_id);
void print_rooms();

#endif
