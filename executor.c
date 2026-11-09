#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void handle_redirection(char *args[]) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i+1], O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        }
    }
}

void run_command(char *args[]) {
    pid_t pid = fork();
    if (pid == 0) {
        handle_redirection(args);
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    } else {
        wait(NULL);
    }
}

void split_pipe_command(char *args[], char *cmd1[], char *cmd2[]) {
    int i, j = 0;
    for (i = 0; args[i] && strcmp(args[i], "|") != 0; i++) {
        cmd1[i] = args[i];
    }
    cmd1[i] = NULL;

    if (args[i] != NULL) i++;
    for (; args[i]; i++) {
        cmd2[j++] = args[i];
    }
    cmd2[j] = NULL;
}

void run_pipe_command(char *cmd1[], char *cmd2[]) {
    int pipefd[2];
    pipe(pipefd);

    if (fork() == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        execvp(cmd1[0], cmd1);
        perror("exec");
        exit(1);
    }

    if (fork() == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        execvp(cmd2[0], cmd2);
        perror("exec");
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL);
    wait(NULL);
}
