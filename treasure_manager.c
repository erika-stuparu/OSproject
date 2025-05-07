#include "treasure.h"



void log_operation(const char *hunt_id, const char *operation) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", LOG_FILE, hunt_id);

    int fd=open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if(fd<0) return;
    char log[512];
    time_t now = time(NULL);
    snprintf(log, sizeof(log), "%s: %s\n", ctime(&now), operation);
    write(fd, log, strlen(log));
    close(fd);

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);
    symlink(path, symlink_name);
}

int add_treasure(const char *hunt_id){
    mkdir(hunt_id, 0755);
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);

    Treasure t;
    printf("Enter ID: ");
    scanf("%d", &t.id);
    printf("Enter username: ");
    scanf("%s", t.username);
    printf("Enter latitude: ");
    scanf("%f", &t.latitude);
    printf("Enter longitude: ");
    scanf("%f", &t.longitude);
    printf("Enter clue: ");
    getchar(); 
    fgets(t.clue, MAX_CLUE, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;
    printf("Enter value: ");
    scanf("%d", &t.value);

    int fd=open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if(fd<0){
        perror("Error opening file");
        return -1;
    }

    write(fd, &t, sizeof(Treasure));
    close(fd);
    
    log_operation(hunt_id, "Added treasure");
    return 0;
}

int list_treasures(const char *hunt_id){
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);

    struct stat st;
    if(stat(path, &st) < 0){
        perror("Error getting hunt");
        return -1;
    }
    printf("Hunt: %s\n",hunt_id);
    printf("Size: %ld bytes\n", st.st_size);
    printf("Last modified: %s", ctime(&st.st_mtime));


    int fd=open(path, O_RDONLY);
    if(fd<0){
        perror("Error opening file");
        return -1;
    }
    Treasure t;
    while(read(fd, &t, sizeof(Treasure))==sizeof(Treasure)){
        printf("ID: %d\n", t.id);
        printf("Username: %s\n", t.username);
        printf("Latitude: %.2f\n", t.latitude);
        printf("Longitude: %.2f\n", t.longitude);
        printf("Clue: %s\n", t.clue);
        printf("Value: %d\n", t.value);
    }
    close(fd);
    log_operation(hunt_id, "Listed treasures");
    return 0;
}

int view_treasure(const char *hunt_id, int target_id){
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd=open(path, O_RDONLY);
    if(fd<0){
        perror("Error opening file");
        return -1;
    }
    Treasure t;
    while(read(fd, &t, sizeof(Treasure))==sizeof(Treasure)){
        if(t.id==target_id){
            printf("ID: %d\n", t.id);
            printf("Username: %s\n", t.username);
            printf("Latitude: %.2f\n", t.latitude);
            printf("Longitude: %.2f\n", t.longitude);
            printf("Clue: %s\n", t.clue);
            printf("Value: %d\n", t.value);
            close(fd);
            log_operation(hunt_id, "Viewed treasure");
            return 0;
        }
    }
    close(fd);
    printf("Treasure not found\n");
    return -1;
}

int remove_treasure(const char *hunt_id, int target_id){
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);

    int fd=open(path, O_RDONLY);
    if(fd<0){
        perror("Error opening file");
        return -1;
    }

    int temp_fd=open("temp_treasure", O_WRONLY | O_CREAT|O_TRUNC, 0644);
    Treasure t;
    int found=0;
    while(read(fd, &t, sizeof(Treasure))==sizeof(Treasure)){
        if(t.id!=target_id){
            write(temp_fd, &t, sizeof(Treasure));
        }else{
            found=1;
        }
    }
    close(fd);
    close(temp_fd);
    rename("temp_treasure", path);

    if(found){
        log_operation(hunt_id, "Removed treasure");
        return 0;
    }else{  
        printf("Treasure not found\n");
       
    }
     return -1;
}

int remove_hunt(const char *hunt_id){
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", hunt_id, TREASURE_FILE);
    unlink(path);
    snprintf(path, sizeof(path), "%s/%s", hunt_id, LOG_FILE);
    unlink(path);
    rmdir(hunt_id);

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);
    log_operation(hunt_id, "Removed hunt");
    return 0;
}

int main(int argc, char *argv[]){
    if(argc<3){
        printf("Usage: %s <operation> <hunt_id> [<target_id>]\n", argv[0]);
        return 1;
    }

    const char *operation = argv[1];
    const char *hunt_id = argv[2];
    int target_id = -1;
    if(argc==4){
        target_id = atoi(argv[3]);
    }

    if(strcmp(operation, "add")==0){
        return add_treasure(hunt_id);
    }else if(strcmp(operation, "list")==0 || strcmp(operation, "list_treasures") == 0){
        return list_treasures(hunt_id);
    }else if(strcmp(operation, "view")==0 || strcmp(operation, "view_treasure") == 0){
        return view_treasure(hunt_id, atoi(argv[3]));
    
    }else if(strcmp(operation, "remove_treasure")==0&&argc==4){
        return remove_treasure(hunt_id, atoi(argv[3]));
    }else if(strcmp(operation, "remove_hunt")==0){
        return remove_hunt(hunt_id);
    }else{
        fprintf(stderr,"Invalid operation");
        return 1;
    }
    

}
