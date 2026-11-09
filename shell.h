#ifndef SHELL_H
#define SHELL_H

#define MAX_LINE 1024
#define MAX_ARGS 128

// for !!
extern char last_command[MAX_LINE];

// functions declarations
void parse_input(char *input, char *args[], char **mode);
void handle_builtin(char *args[]);
void run_command(char *args[]);
void handle_redirection(char *args[]);
void run_pipe_command(char *cmd1[], char *cmd2[]);
void split_pipe_command(char *args[], char *cmd1[], char *cmd2[]);

#endif
