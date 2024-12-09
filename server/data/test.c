#include "rooms.h"

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

int main() {
    add_room("First Room", "creator1");
    add_room("Second Room", "creator2");

    add_user_to_room(1, "player1");
    add_user_to_room(1, "player2");
    add_user_to_room(1, "player3");
    add_user_to_room(1, "player4");

    print_rooms();

    remove_user_from_room(1, "creator1");
    remove_user_from_room(2, "creator2");

    print_rooms();

    return 0;
}
