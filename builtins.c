#include "shell.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

void handle_builtin(char *args[]) {
    if (strcmp(args[0], "cd") == 0) {
        chdir(args[1]);
    } else if (strcmp(args[0], "mkdir") == 0) {
        mkdir(args[1], 0755);
    } else if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands: cd, mkdir, help, exit, !!\n");
    }
}
