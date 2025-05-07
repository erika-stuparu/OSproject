#include "treasure.h"

pid_t monitor_pid = 0;
int shutdown = 0;
int stop = 0;

void monitor_exit(int sig){
    int status;
    pid_t ended_pid=waitpid(monitor_pid, &status, WNOHANG);
    if(ended_pid==monitor_pid){
        if (WIFEXITED(status)){
            printf("Monitor process %d exited with status %d\n", monitor_pid, WEXITSTATUS(status));
        }else if(WIFSIGNALED(status)){
            printf("Monitor process %d ended by signal %d\n", monitor_pid, WTERMSIG(status));
        }
        monitor_pid=0;
        shutdown=0;
    }
}

void signal_wakeup(int sig){}

void signal_stop(int sig){
    stop=1;
}

void save_command(const char *cmd){
    FILE *file = fopen(CMD_FILE, "w");
    if(!file){
        perror("Error opening command file");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s\n", cmd);
    fclose(file);
}

void signal_monitor(int sig_number){
    if(monitor_pid>0){
        kill(monitor_pid, sig_number);
    }
}
    
pid_t start_monitor(void) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error starting monitor process");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        struct sigaction sa_usr1 = {0}, sa_usr2 = {0};
        sigset_t wait_mask;

        sa_usr1.sa_handler = signal_wakeup;
        sigaction(SIGUSR1, &sa_usr1, NULL);

        sa_usr2.sa_handler = signal_stop;
        sigaction(SIGUSR2, &sa_usr2, NULL);

        sigemptyset(&wait_mask);

        while (!stop) {
            sigsuspend(&wait_mask);
            if (stop) break;

            FILE *file = fopen(CMD_FILE, "r");
            if (!file) {
                perror("Error opening command file");
                continue;
            }

            char file_cmd[256];
            if (fgets(file_cmd, sizeof(file_cmd), file) != NULL) {
                file_cmd[strcspn(file_cmd, "\n")] = 0;

                if (strcmp(file_cmd, "list_hunts") == 0) {
                    DIR *dir = opendir(".");
                    if (dir == NULL) {
                        perror("Error opening directory");
                        fclose(file);
                        continue;
                    }

                    struct dirent *entry;
                    while ((entry = readdir(dir))) {
                        if (entry->d_type == DT_DIR &&
                            strcmp(entry->d_name, ".") != 0 &&
                            strcmp(entry->d_name, "..") != 0) {

                            char treasure_file_path[256];
                            snprintf(treasure_file_path, sizeof(treasure_file_path), "%s/%s", entry->d_name, TREASURE_FILE);
                            struct stat st;
                            if (stat(treasure_file_path, &st) == 0) {
                                int treasure_count = st.st_size / sizeof(Treasure);
                                printf("Hunt: %-12s | Treasures: %d\n", entry->d_name, treasure_count);
                            }
                        }
                    }
                    closedir(dir);
                } else if (strncmp(file_cmd, "list_treasures", 4) == 0 ||
                           strncmp(file_cmd, "view_treasures", 4) == 0) {
                    char full_cmd[512];
                    snprintf(full_cmd, sizeof(full_cmd), "./treasure_manager %s", file_cmd);
                    system(full_cmd);
                } else {
                    printf("Unknown command: %s\n", file_cmd);
                }
            }

            fclose(file);
        }

        usleep(500000);
        printf("Shutting down.\n");
        exit(0);
    }

    return pid;
}

int main(void) {
    char input[256];

    struct sigaction chld_action = {0};
    chld_action.sa_handler = monitor_exit;
    sigaction(SIGCHLD, &chld_action, NULL);

    printf("treasure_hub> ");

    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0;

        if (stop && strcmp(input, "exit") != 0) {
            printf("Monitor is shutting down. \n");
        } else if (strcmp(input, "start_monitor") == 0) {
            if (monitor_pid) {
                printf("Monitor already running (PID %d)\n", monitor_pid);
            } else {
                monitor_pid = start_monitor();
                printf("Monitor started (PID %d)\n", monitor_pid);
            }
        } else if (strcmp(input, "stop_monitor") == 0) {
            if (!monitor_pid) {
                printf("No monitor is running.\n");
            } else {
                signal_monitor(SIGUSR2);
                printf("Monitor shutting down.\n");
            }
        } else if (strcmp(input, "exit") == 0) {
            if (monitor_pid) {
                printf("Cannot exit while monitor is running.\n");
            } else {
                break;
            }
        } else if (strncmp(input, "list_hunts", 10) == 0 ||
                   strncmp(input, "list_treasures", 14) == 0 ||
                   strncmp(input, "view_treasure", 13) == 0) {
            if (!monitor_pid) {
                printf("Monitor not running. Use start_monitor first.\n");
            } else {
                save_command(input);
                signal_monitor(SIGUSR1);
            }
        } else {
            printf("Unknown command: %s\n", input);
        }

        printf("treasure_hub> ");
    }


    return 0;
}

    