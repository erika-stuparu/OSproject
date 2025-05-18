#include "treasure.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
        return 1;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/%s", argv[1], TREASURE_FILE);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening treasure file");
        return 1;
    }

    Treasure t;
    struct {
        char username[MAX_USERNAME];
        int score;
    } users[100];

    int user_count = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        int found = 0;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].username, t.username) == 0) {
                users[i].score += t.value;
                found = 1;
                break;
            }
        }
        if (!found && user_count < 100) {
            strncpy(users[user_count].username, t.username, MAX_USERNAME);
            users[user_count].score = t.value;
            user_count++;
        }
    }

    for (int i = 0; i < user_count; i++) {
        printf("User: %s | Score: %d\n", users[i].username, users[i].score);
    }

    close(fd);
    return 0;
}
