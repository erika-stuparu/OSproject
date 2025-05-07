#ifndef TREASURE_H
#define TREASURE_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>


#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>


#define MAX_USERNAME   32
#define MAX_CLUE      128
#define TREASURE_FILE  "treasures.dat"
#define LOG_FILE       "logged_hunt"
#define CMD_FILE       "hub_cmd.txt"

typedef struct {
    int   id;
    char  username[MAX_USERNAME];
    float latitude;
    float longitude;
    char  clue[MAX_CLUE];
    int   value;
} Treasure;


int  add_treasure(const char *hunt_id);
int  list_treasures(const char *hunt_id);
int  view_treasure(const char *hunt_id, int tid);
int  remove_treasure(const char *hunt_id, int tid);
int  remove_hunt(const char *hunt_id);
void log_operation(const char *hunt_id, const char *operation);


void monitor_exit(int sig);
void signal_wakeup(int sig);
void signal_stop(int sig);
void save_command(const char *cmd);
void signal_monitor(int sig_number);
pid_t start_monitor(void);


#endif /* TREASURE_H */
