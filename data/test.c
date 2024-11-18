#include <stdio.h>
#include "data.h"

int main() {
    const char *filename = "user.txt";
    const char *usernameToFind = "hiew1";

    const char *username = "hiew6";
    const char *password = "password123";

    if (login(filename, username, password)) {
        printf("Dang nhap thanh cong.\n");
    } else {
        printf("Sai ten dang nhap hoac mat khau.\n");
    }

    return 0;
}